/*
 * SPDX-FileCopyrightText: Copyright (c) 2023-2024 NVIDIA CORPORATION &
 * AFFILIATES. All rights reserved. SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <phosphor-logging/lg2.hpp>

#include <algorithm>
#include <charconv>
#include <chrono>
#include <cmath>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <ratio>
#include <string>
#include <string_view>
#include <system_error>
#include <tuple>

// IWYU pragma: no_include <stddef.h>
// IWYU pragma: no_include <stdint.h>

using namespace std;

namespace nv
{
namespace sensor_aggregation
{
namespace metricUtils
{
namespace details
{

constexpr intmax_t dayDuration = static_cast<intmax_t>(24 * 60 * 60);
using Days = chrono::duration<long long, ratio<dayDuration>>;

/**
 * @brief Creates a string from an integer in the most efficient way possible
 * without using locale.  Adds an exact zero pad based on the pad input
 * parameter. Does not handle negative numbers.
 *
 * @param[in] value
 * @param[in] pad
 * @return string
 */
inline string padZeros(int64_t value, size_t pad)
{
    string result(pad, '0');
    for (int64_t val = value; pad > 0; pad--)
    {
        result[pad - 1] = static_cast<char>('0' + val % 10);
        val /= 10;
    }
    return result;
}

/**
 * @brief Method which formats duration item.
 *
 * @tparam FromTime
 * @param[in] fmt
 * @param[in] postfix
 * @param[out] out
 * @return true
 * @return false
 */
template <typename FromTime>
bool fromDurationItem(string_view& fmt, const char postfix,
                      chrono::milliseconds& out)
{
    const size_t pos = fmt.find(postfix);
    if (pos == string::npos)
    {
        return true;
    }
    if ((pos + 1U) > fmt.size())
    {
        return false;
    }

    const char* end = nullptr;
    chrono::milliseconds::rep ticks = 0;
    if constexpr (is_same_v<FromTime, chrono::milliseconds>)
    {
        end = fmt.data() + min<size_t>(pos, 3U);
    }
    else
    {
        end = fmt.data() + pos;
    }

    auto [ptr, ec] = from_chars(fmt.data(), end, ticks);
    if (ptr != end || ec != errc())
    {
        lg2::error("SHMEMDEBUG: Failed to convert string to decimal with err: "
                   "{CONV_ERROR}",
                   "CONV_ERROR", static_cast<int>(ec));
        string errorMessage =
            "SHMEMDEBUG: Failed to convert string to decimal with err: " +
            to_string(static_cast<int>(ec));
        LOG_ERROR(errorMessage);
        return false;
    }

    if constexpr (is_same_v<FromTime, chrono::milliseconds>)
    {
        ticks *= static_cast<chrono::milliseconds::rep>(
            pow(10, 3 - min<size_t>(pos, 3U)));
    }
    if (ticks < 0)
    {
        return false;
    }

    out += FromTime(ticks);
    const auto maxConversionRange =
        chrono::duration_cast<FromTime>(chrono::milliseconds::max()).count();
    if (out < FromTime(ticks) || maxConversionRange < ticks)
    {
        return false;
    }

    fmt.remove_prefix(pos + 1U);
    return true;
}
} // namespace details

/**
 * @brief Convert string that represents value in Duration Format to its numeric
 *        equivalent.
 *
 * @param str
 * @return optional<chrono::milliseconds>
 */
inline optional<chrono::milliseconds> fromDurationString(const string& str)
{
    chrono::milliseconds out = chrono::milliseconds::zero();
    string_view v = str;

    if (v.empty())
    {
        return out;
    }
    if (v.front() != 'P')
    {
        string errorMessage = "SHMEMDEBUG: Invalid duration format: " + str;
        LOG_ERROR(errorMessage);
        return nullopt;
    }

    v.remove_prefix(1);
    if (!details::fromDurationItem<details::Days>(v, 'D', out))
    {
        string errorMessage = "SHMEMDEBUG: Invalid duration format: " + str;
        LOG_ERROR(errorMessage);
        return nullopt;
    }

    if (v.empty())
    {
        return out;
    }
    if (v.front() != 'T')
    {
        string errorMessage = "SHMEMDEBUG: Invalid duration format: " + str;
        LOG_ERROR(errorMessage);
        return nullopt;
    }

    v.remove_prefix(1);
    if (!details::fromDurationItem<chrono::hours>(v, 'H', out) ||
        !details::fromDurationItem<chrono::minutes>(v, 'M', out))
    {
        string errorMessage = "SHMEMDEBUG: Invalid duration format: " + str;
        LOG_ERROR(errorMessage);
        return nullopt;
    }

    if (v.find('.') != string::npos && v.find('S') != string::npos)
    {
        if (!details::fromDurationItem<chrono::seconds>(v, '.', out) ||
            !details::fromDurationItem<chrono::milliseconds>(v, 'S', out))
        {
            string errorMessage = "SHMEMDEBUG: Invalid duration format: " + str;
            LOG_ERROR(errorMessage);
            return nullopt;
        }
    }
    else if (!details::fromDurationItem<chrono::seconds>(v, 'S', out))
    {
        string errorMessage = "SHMEMDEBUG: Invalid duration format: " + str;
        LOG_ERROR(errorMessage);
        return nullopt;
    }

    if (!v.empty())
    {
        string errorMessage = "SHMEMDEBUG: Invalid duration format: " + str;
        LOG_ERROR(errorMessage);
        return nullopt;
    }
    return out;
}

/**
 * @brief Convert time value into duration format that is based on ISO 8601.
 *        Example output: "P12DT1M5.5S"
 *        Ref: Redfish Specification, Section 9.4.4. Duration values *
 * @param[in] ms - time in milliseconds
 * @return string
 */
inline string toDurationString(chrono::milliseconds ms)
{
    if (ms < chrono::milliseconds::zero())
    {
        return "";
    }

    string fmt;
    fmt.reserve(sizeof("PxxxxxxxxxxxxDTxxHxxMxx.xxxxxxS"));

    details::Days days = chrono::floor<details::Days>(ms);
    ms -= days;

    chrono::hours hours = chrono::floor<chrono::hours>(ms);
    ms -= hours;

    chrono::minutes minutes = chrono::floor<chrono::minutes>(ms);
    ms -= minutes;

    chrono::seconds seconds = chrono::floor<chrono::seconds>(ms);
    ms -= seconds;

    fmt = "P";
    if (days.count() > 0)
    {
        fmt += to_string(days.count()) + "D";
    }
    fmt += "T";
    if (hours.count() > 0)
    {
        fmt += to_string(hours.count()) + "H";
    }
    if (minutes.count() > 0)
    {
        fmt += to_string(minutes.count()) + "M";
    }
    if (seconds.count() != 0 || ms.count() != 0)
    {
        fmt += to_string(seconds.count()) + ".";
        fmt += details::padZeros(ms.count(), 3);
        fmt += "S";
    }
    else if (fmt == "PT")
    {
        fmt += "0S"; // Append "0S" when time is zero seconds
    }

    return fmt;
}

/**
 * @brief Method to Convert time value into string from interger time(epoch
 * time)
 * @param[in] ms - time in milliseconds
 * @return string
 */
inline optional<string> toDurationStringFromUint(const uint64_t timeMs)
{
    static const uint64_t maxTimeMs =
        static_cast<uint64_t>(chrono::milliseconds::max().count());

    if (maxTimeMs < timeMs)
    {
        return nullopt;
    }

    string duration = toDurationString(chrono::milliseconds(timeMs));
    if (duration.empty())
    {
        return nullopt;
    }

    return make_optional(duration);
}

namespace details
{
// Returns year/month/day triple in civil calendar
// Preconditions:  z is number of days since 1970-01-01 and is in the range:
//                   [numeric_limits<Int>::min(),
//                   numeric_limits<Int>::max()-719468].
// Algorithm sourced from
// https://howardhinnant.github.io/date_algorithms.html#civil_from_days
// All constants are explained in the above
template <class IntType>
constexpr tuple<IntType, unsigned, unsigned> civilFromDays(IntType z) noexcept
{
    z += 719468;
    IntType era = (z >= 0 ? z : z - 146096) / 146097;
    unsigned doe = static_cast<unsigned>(z - era * 146097); // [0, 146096]
    unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) /
                   365;                                     // [0, 399]
    IntType y = static_cast<IntType>(yoe) + era * 400;
    unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100); // [0, 365]
    unsigned mp = (5 * doy + 2) / 153;                      // [0, 11]
    unsigned d = doy - (153 * mp + 2) / 5 + 1;              // [1, 31]
    unsigned m = mp < 10 ? mp + 3 : mp - 9;                 // [1, 12]

    return tuple<IntType, unsigned, unsigned>(y + (m <= 2), m, d);
}

/**
 * @brief Returns timstring in ISO8061 extended string format.
 *
 * @tparam IntType
 * @tparam[in] Period
 * @param[in] t - time in duration
 * @return string
 */
template <typename IntType, typename Period>
string toISO8061ExtendedStr(chrono::duration<IntType, Period> t)
{
    using seconds = chrono::duration<int>;
    using minutes = chrono::duration<int, ratio<60>>;
    using hours = chrono::duration<int, ratio<3600>>;
    using days =
        chrono::duration<IntType, ratio_multiply<hours::period, ratio<24>>>;

    // d is days since 1970-01-01
    days d = chrono::duration_cast<days>(t);

    // t is now time duration since midnight of day d
    t -= d;

    // break d down into year/month/day
    int year = 0;
    int month = 0;
    int day = 0;
    tie(year, month, day) = details::civilFromDays(d.count());
    // Check against limits.  Can't go above year 9999, and can't go below epoch
    // (1970)
    if (year >= 10000)
    {
        year = 9999;
        month = 12;
        day = 31;
        t = days(1) - chrono::duration<IntType, Period>(1);
    }
    else if (year < 1970)
    {
        year = 1970;
        month = 1;
        day = 1;
        t = chrono::duration<IntType, Period>::zero();
    }

    string out;
    out += details::padZeros(year, 4);
    out += '-';
    out += details::padZeros(month, 2);
    out += '-';
    out += details::padZeros(day, 2);
    out += 'T';
    hours hr = chrono::duration_cast<hours>(t);
    out += details::padZeros(hr.count(), 2);
    t -= hr;
    out += ':';

    minutes mt = chrono::duration_cast<minutes>(t);
    out += details::padZeros(mt.count(), 2);
    t -= mt;
    out += ':';

    seconds se = chrono::duration_cast<seconds>(t);
    out += details::padZeros(se.count(), 2);
    t -= se;

    if constexpr (is_same_v<typename decltype(t)::period, milli>)
    {
        out += '.';
        using MilliDuration = chrono::duration<int, milli>;
        MilliDuration subsec = chrono::duration_cast<MilliDuration>(t);
        out += details::padZeros(subsec.count(), 3);
    }
    else if constexpr (is_same_v<typename decltype(t)::period, micro>)
    {
        out += '.';

        using MicroDuration = chrono::duration<int, micro>;
        MicroDuration subsec = chrono::duration_cast<MicroDuration>(t);
        out += details::padZeros(subsec.count(), 6);
    }

    out += "+00:00";
    return out;
}
} // namespace details

/**
 * @brief Returns the formatted date time string. Note that the maximum
 * supported date is 9999-12-31T23:59:59+00:00, if the given |secondsSinceEpoch|
 * is too large, we return the maximum supported date.
 *
 * @param[in] secondsSinceEpoch
 * @return string
 */
inline string getDateTimeUint(uint64_t secondsSinceEpoch)
{
    using DurationType = chrono::duration<uint64_t>;
    DurationType sinceEpoch(secondsSinceEpoch);
    return details::toISO8061ExtendedStr(sinceEpoch);
}

/**
 * @brief Returns the formatted date time string with millisecond precision.
 * Note that the maximum supported date is 9999-12-31T23:59:59+00:00, if the
 * given |secondsSinceEpoch| is too large, we return the maximum supported date.
 *
 * @param[in] milliSecondsSinceEpoch
 * @return string
 */
inline string getDateTimeUintMs(uint64_t milliSecondsSinceEpoch)
{
    using DurationType = chrono::duration<uint64_t, milli>;
    DurationType sinceEpoch(milliSecondsSinceEpoch);
    return details::toISO8061ExtendedStr(sinceEpoch);
}

/**
 * @brief Returns the formatted date time string with microsecond precision
 *
 * @param[in] microSecondsSinceEpoch
 * @return string
 */
inline string getDateTimeUintUs(uint64_t microSecondsSinceEpoch)
{
    using DurationType = chrono::duration<uint64_t, micro>;
    DurationType sinceEpoch(microSecondsSinceEpoch);
    return details::toISO8061ExtendedStr(sinceEpoch);
}

/**
 * @brief Returns the formatted date time string in std time format.
 *
 * @param[in] secondsSinceEpoch
 * @return string
 */
inline string getDateTimeStdtime(time_t secondsSinceEpoch)
{
    using DurationType = chrono::duration<time_t>;
    DurationType sinceEpoch(secondsSinceEpoch);
    return details::toISO8061ExtendedStr(sinceEpoch);
}

/**
 * Returns the current Date, Time & the local Time Offset
 * infromation in a pair
 *
 * @param[in] None
 *
 * @return pair<string, string>, which consist
 * of current DateTime & the TimeOffset strings respectively.
 */
inline pair<string, string> getDateTimeOffsetNow()
{
    time_t tmpTime = time(nullptr);
    string dateTime = getDateTimeStdtime(tmpTime);

    /* extract the local Time Offset value from the
     * recevied dateTime string.
     */
    string timeOffset("Z00:00");
    size_t lastPos = dateTime.size();
    size_t len = timeOffset.size();
    if (lastPos > len)
    {
        timeOffset = dateTime.substr(lastPos - len);
    }

    return make_pair(dateTime, timeOffset);
}

inline time_t getTimestamp(uint64_t millisTimeStamp)
{
    // Retrieve Created property with format:
    // yyyy-mm-ddThh:mm:ss
    chrono::milliseconds chronoTimeStamp(millisTimeStamp);
    return chrono::duration_cast<chrono::duration<int>>(chronoTimeStamp)
        .count();
}

inline string nanoSecToDurationString(chrono::nanoseconds ns)
{
    if (ns < chrono::nanoseconds::zero())
    {
        return "";
    }

    string fmt;
    fmt.reserve(sizeof("PxxxxxxxxxxxxDTxxHxxMxx.xxxxxxS"));

    details::Days days = chrono::floor<details::Days>(ns);
    ns -= days;

    chrono::hours hours = chrono::floor<chrono::hours>(ns);
    ns -= hours;

    chrono::minutes minutes = chrono::floor<chrono::minutes>(ns);
    ns -= minutes;

    chrono::seconds seconds = chrono::floor<chrono::seconds>(ns);
    ns -= seconds;

    fmt = "P";
    if (days.count() > 0)
    {
        fmt += to_string(days.count()) + "D";
    }
    fmt += "T";
    if (hours.count() > 0)
    {
        fmt += to_string(hours.count()) + "H";
    }
    if (minutes.count() > 0)
    {
        fmt += to_string(minutes.count()) + "M";
    }

    if (seconds.count() != 0 || ns.count() != 0)
    {
        fmt += to_string(seconds.count()) + ".";
        fmt += details::padZeros(ns.count(), 9);
        fmt += "S";
    }
    else if (fmt == "PT")
    {
        fmt += "0S"; // Append "0S" when time is zero seconds
    }

    return fmt;
}

inline optional<string> toDurationStringFromNano(const uint64_t timeNs)
{
    static const uint64_t maxTimeNs =
        static_cast<uint64_t>(chrono::nanoseconds::max().count());

    if (maxTimeNs < timeNs)
    {
        return nullopt;
    }

    chrono::nanoseconds nanosecs(timeNs);

    string duration = nanoSecToDurationString(nanosecs);
    if (duration.empty())
    {
        return nullopt;
    }

    return make_optional(duration);
}

} // namespace metricUtils
} // namespace sensor_aggregation
} // namespace nv
