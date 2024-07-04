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

#include "config.h"

#include "telemetry_mrd_client.hpp"

#include "impl/config_json_reader.hpp"
#include "impl/shm_sensormap_intf.hpp"
#include "impl/shmem_map.hpp"

#include <phosphor-logging/lg2.hpp>

#include <string>
#include <unordered_map>

using namespace std;

namespace nv
{
namespace shmem
{
namespace sensor_aggregation
{

ShmemKeyValuePairs getAllKeyValuePair(const std::string& mrdNamespace)
{
    static unordered_map<string, unique_ptr<sensor_map_type>> sensor_map;
    try
    {
        if (sensor_map.find(mrdNamespace) == sensor_map.end())
        {
            sensor_map.insert(std::make_pair(
                mrdNamespace, make_unique<sensor_map_type>(mrdNamespace, O_RDONLY)));
        }
        return sensor_map[mrdNamespace]->getAllKeyValuePair();
    }
    catch (const exception& e)
    {
        lg2::error("SHMEMDEBUG: Exception {EXCEPTION} while reading from {MRD} "
                   "namespace",
                   "EXCEPTION", e.what(), "MRD", mrdNamespace);
        throw NameSpaceNotFoundException();           
    }
    throw NoElementsException();
}

vector<SensorValue> getAllMRDValues(const string& mrdNamespace)
{
    static unordered_map<string, unique_ptr<sensor_map_type>> sensor_map;
    static unordered_map<string, vector<string>> mrdNamespaceLookup =
        ConfigReader::getMRDNamespaceLookup();
    vector<SensorValue> values;
    if (mrdNamespaceLookup.find(mrdNamespace) != mrdNamespaceLookup.end())
    {
        for (auto& producerName : mrdNamespaceLookup[mrdNamespace])
        {
            try
            {
                auto nameSpace = producerName + "_" + mrdNamespace;
                if (sensor_map.find(nameSpace) == sensor_map.end())
                {
                    sensor_map.insert(std::make_pair(
                        nameSpace,
                        make_unique<sensor_map_type>(nameSpace, O_RDONLY)));
                }
                const auto& mrdValues = sensor_map[nameSpace]->getAllValues();
                if (mrdValues.size())
                {
                    SHMDEBUG(
                        "SHMEMDEBUG: Requested {MRD} namespace has {NUMBER} of elements",
                        "MRD", nameSpace, "NUMBER", mrdValues.size());
                    values.insert(values.end(), mrdValues.begin(),
                                  mrdValues.end());
                }
                else
                {
                    lg2::error(
                        "SHMEMDEBUG: Requested {MRD} namespace has no elements",
                        "MRD", nameSpace);
                }
            }
            catch (const exception& e)
            {
                lg2::error(
                    "SHMEMDEBUG: Exception {EXCEPTION} while reading from {MRD} "
                    "namespace",
                    "EXCEPTION", e.what(), "MRD", mrdNamespace);
            }
        }
        if (values.size() != 0)
        {
            return values;
        }
        else
        {
            lg2::error("SHMEMDEBUG: Requested {MRD} namespace has no elements.",
                       "MRD", mrdNamespace);
            throw NoElementsException();
        }
    }
    else
    {
        lg2::error(
            "SHMEMDEBUG: Requested {MRD} namespace is not found in the MRD lookup.",
            "MRD", mrdNamespace);
        throw NameSpaceNotFoundException();
    }
}

vector<string> getMrdNamespacesValues()
{
    static unordered_map<string, vector<string>> mrdNamespaceLookup =
        ConfigReader::getMRDNamespaceLookup();
    vector<string> mrd;
    for (const auto& pair : mrdNamespaceLookup)
    {
        mrd.push_back(pair.first);
    }
    return mrd;
}

} // namespace sensor_aggregation
} // namespace shmem
} // namespace nv
