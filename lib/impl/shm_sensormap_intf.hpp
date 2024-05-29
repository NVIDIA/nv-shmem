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
#include "shmem_map.hpp"

#include <boost/format.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <phosphor-logging/lg2.hpp>

#include <iomanip>
#include <memory>
#include <unordered_map>

using namespace std;
using namespace nv::shmem;
using sensor_map_type =
    nv::shmem::Map<nv::shmem::SensorMap, nv::shmem::SensorValue>;
namespace nv
{
namespace shmem
{

class ShmSensorMapIntf
{
  public:
    ShmSensorMapIntf() {}
    ~ShmSensorMapIntf() {}

    /**
     * @brief Checks whether namespace is present in shared memory or not.
     *
     * @param[in] nameSpace
     * @return true
     * @return false
     */
    bool isNameSpacePresent(const string& nameSpace)
    {
        if (sensor_map.find(nameSpace) == sensor_map.end())
        {
            return false;
        }
        return true;
    }

    /**
     * @brief Create shared memory namespace in shared memory with specified
     * size and permissions.
     *
     * @param[in] nameSpace - shared memory namespace name
     * @param[in] shmSize - shared memory size in bytes
     */
    bool createNamespace(const string& nameSpace, const size_t shmSize)
    {
        try
        {
            sensor_map.insert(std::make_pair(
                nameSpace,
                make_unique<sensor_map_type>(nameSpace, O_CREAT, shmSize)));
        }
        catch (const exception& e)
        {
            lg2::error(
                "SHMEMDEBUG: ShmSensorMapIntf init Exception: {INITERROR}",
                "INITERROR", e.what());
            return false;
        }
        return true;
    }

    /**
     * @brief Insert new object into shared memory.
     *
     * @param[in] mrdNamespace - mrd namespace
     * @param[in] key - shared memory key
     * @param[in] value - shared memory value
     * @return true
     * @return false
     */
    bool insert(const string& mrdNamespace, const string& key,
                const SensorValue& value)
    {
        try
        {
            auto itr = sensor_map.find(mrdNamespace);
            if (itr != sensor_map.end())
            {
                (*itr).second->insert(key, value);
                return true;
            }
            else
            {
                lg2::error(
                    "SHMEMDEBUG: ShmSensorMapIntf insert unknown name space: "
                    "{SHM_NAMESPACE}",
                    "SHM_NAMESPACE", mrdNamespace);
                return false;
            }
        }
        catch (const exception& e)
        {
            lg2::error(
                "SHMEMDEBUG: ShmSensorMapIntf insert Exception: {SHM_EXCEPTION}",
                "SHM_EXCEPTION", e.what());
            return false;
        }
    }

    /**
     * @brief update timestamp in shared memory
     *
     * @param[in] mrdNamespace - shared memory namespace
     * @param[in] key - shared memory key
     * @param[in] timestamp - timestamp in epoch.
     * @param[in] timeStampStr - timestamp string value in redfish ISO format
     * @return true
     * @return false
     */
    bool updateTimeStamp(const string& mrdNamespace, const string& key,
                         const uint64_t timestamp, const string& timeStampStr)
    {
        try
        {
            auto itr = sensor_map.find(mrdNamespace);
            if (itr != sensor_map.end())
            {
                if (!((*itr).second->updateTimestamp(key, timestamp,
                                                     timeStampStr)))
                {
                    lg2::error(
                        "SHMEMDEBUG: Invalid shared memory key: {SHMKEY}",
                        "SHMKEY", key);
                    return false;
                }
                return true;
            }
            else
            {
                lg2::error(
                    "SHMEMDEBUG: ShmSensorMapIntf updateTimestamp unknown name "
                    "space: {SHM_NAMESPACE}",
                    "SHM_NAMESPACE", mrdNamespace);
                return false;
            }
        }
        catch (const exception& e)
        {
            lg2::error("SHMEMDEBUG: ShmSensorMapIntf updateTimestamp Exception "
                       ": {SHM_EXCEPTION}",
                       "SHM_EXCEPTION", e.what());
            return false;
        }
    }

    /**
     * @brief update value in shared memory
     *
     * @param[in] mrdNamespace - shared memory namespace
     * @param[in] key - shared memory key
     * @param[in] value - shared memory value
     * @return true
     * @return false
     */
    bool updateValue(const string& mrdNamespace, const string& key,
                     const string& value)
    {
        try
        {
            auto itr = sensor_map.find(mrdNamespace);
            if (itr != sensor_map.end())
            {
                if (!((*itr).second->updateValue(key, value)))
                {
                    lg2::error(
                        "SHMEMDEBUG: Invalid shared memory key: {SHM_KEY}",
                        "SHM_KEY", key);
                    return false;
                }
                return true;
            }
            else
            {
                lg2::error(
                    "SHMEMDEBUG: ShmSensorMapIntf updateValue unknown name "
                    "space: {SHM_NAMESPACE}",
                    "SHM_NAMESPACE", mrdNamespace);
                return false;
            }
        }
        catch (const exception& e)
        {
            lg2::error(
                "SHMEMDEBUG: ShmSensorMapIntf updateValue Exception: {SHM_NAMESPACE}",
                "SHM_NAMESPACE", e.what());
            return false;
        }
    }

    /**
     * @brief update value and timestamp in shared memory
     *
     * @param[in] mrdNamespace - shared memory namespace
     * @param[in] key - shared memory key
     * @param[in] value - shared memory value
     * @param[in] timestamp - timestamp in epoch.
     * @param[in] timeStampStr - timestamp string value in redfish ISO format
     * @return true
     * @return false
     */
    bool updateValueAndTimeStamp(const string& mrdNamespace, const string& key,
                                 const string& value, const uint64_t timestamp,
                                 const string& timeStampStr)
    {
        try
        {
            auto itr = sensor_map.find(mrdNamespace);
            if (itr != sensor_map.end())
            {
                if (!((*itr).second->updateValueAndTimeStamp(
                        key, value, timestamp, timeStampStr)))
                {
                    lg2::error(
                        "SHMEMDEBUG: Invalid shared memory key: {SHM_KEY}",
                        "SHM_KEY", key);
                    return false;
                }
                return true;
            }
            else
            {
                lg2::error(
                    "SHMEMDEBUG: ShmSensorMapIntf updateValueAndTimeStamp unknown name "
                    "space: {SHM_NAMESPACE}",
                    "SHM_NAMESPACE", mrdNamespace);
                return false;
            }
        }
        catch (const exception& e)
        {
            lg2::error("SHMEMDEBUG: ShmSensorMapIntf updateValueAndTimeStamp "
                       "Exception: {SHM_NAMESPACE}",
                       "SHM_NAMESPACE", e.what());
            return false;
        }
    }

    /**
     * @brief erase key in shared memory
     *
     * @param[in] mrdNamespace - shared memory namespace
     * @param[in] key - shared memory key
     * @return true
     * @return false
     */
    bool erase(const string& mrdNamespace, const string& key)
    {
        try
        {
            auto itr = sensor_map.find(mrdNamespace);
            if (itr != sensor_map.end())
            {
                (*itr).second->erase(key);
                return true;
            }
            else
            {
                lg2::error(
                    "SHMEMDEBUG: ShmSensorMapIntf erase unknown name space: "
                    "{SHM_NAMESPACE}",
                    "SHM_NAMESPACE", mrdNamespace);
                return false;
            }
        }
        catch (const exception& e)
        {
            lg2::error(
                "SHMEMDEBUG: ShmSensorMapIntf erase Exception: {SHM_NAMESPACE}",
                "SHM_NAMESPACE", e.what());
            return false;
        }
    }

  private:
    unordered_map<string, unique_ptr<sensor_map_type>> sensor_map;
};
} // namespace shmem
} // namespace nv
