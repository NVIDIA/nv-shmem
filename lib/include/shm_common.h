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
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <sdbusplus/bus.hpp>

#include <vector>

#ifdef ENABLE_SHM_DEBUG
#define SHMDEBUG(msg, ...) lg2::info(msg, __VA_ARGS__)
#else
#define SHMDEBUG(msg, ...)
#endif

namespace nv
{
namespace sensor_aggregation
{
// using namespace std;
using DbusVariantType = std::variant<
    std::vector<std::tuple<std::string, std::string, std::string>>,
    std::vector<std::string>, std::vector<double>, std::string, int64_t,
    uint64_t, double, int32_t, uint32_t, int16_t, uint16_t, uint8_t, bool,
    sdbusplus::message::unix_fd, std::vector<uint32_t>, std::vector<uint16_t>,
    sdbusplus::message::object_path,
    std::tuple<uint64_t, std::vector<std::tuple<std::string, std::string,
                                                double, uint64_t>>>,
    std::vector<std::tuple<std::string, std::string>>,
    std::vector<std::tuple<uint32_t, std::vector<uint32_t>>>,
    std::vector<std::tuple<uint32_t, size_t>>,
    std::vector<std::tuple<sdbusplus::message::object_path, std::string,
                           std::string, std::string>>,
    std::vector<sdbusplus::message::object_path>, std::vector<uint8_t>,
    std::vector<std::tuple<uint8_t, std::string>>, std::tuple<size_t, bool>,
    std::tuple<bool, uint32_t>, std::map<std::string, uint64_t>,
    std::tuple<std::string, std::string, std::string, uint64_t>>;
} // namespace sensor_aggregation
} // namespace nv

namespace nv
{
namespace shmem
{
using segment_manager_t =
    boost::interprocess::managed_shared_memory::segment_manager;
using void_allocator_t =
    boost::interprocess::allocator<void, segment_manager_t>;
using char_allocator_t =
    boost::interprocess::allocator<char, segment_manager_t>;
/* Many implementations of the standard library are not flexible enough to use
 * containers such as std::string or std::list with Boost.Interprocess. So
 * boost::interprocess::basic_string is preferred. */
using char_string_t =
    boost::container::basic_string<char, std::char_traits<char>,
                                   char_allocator_t>;
struct SensorMapValue
{
    char_string_t sensorValue;
    char_string_t timestampStr;
    char_string_t metricProperty;
    uint64_t timestamp;

    SensorMapValue(const void_allocator_t& void_alloc) :
        sensorValue(void_alloc), timestampStr(void_alloc),
        metricProperty(void_alloc), timestamp(0U)
    {}
};

struct SensorValue
{
    std::string sensorValue;
    std::string metricProperty;
    uint64_t timestamp;
    std::string timestampStr;

    SensorValue() = default;

    SensorValue(const std::string& sensorValue,
                const std::string& metricProperty, const uint64_t timestamp,
                const std::string& timestampStr) :
        sensorValue(sensorValue), metricProperty(metricProperty),
        timestamp(timestamp), timestampStr(timestampStr)
    {}

    SensorValue& operator=(const SensorMapValue& mapValue)
    {
        metricProperty = mapValue.metricProperty;
        timestampStr = mapValue.timestampStr;
        sensorValue = mapValue.sensorValue;
        timestamp = mapValue.timestamp;
        return *this;
    }
};

} // namespace shmem
} // namespace nv
