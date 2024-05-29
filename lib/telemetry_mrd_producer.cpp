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

#include "telemetry_mrd_producer.hpp"

#include "impl/config_json_reader.hpp"
#include "impl/shm_sensor_aggregator.hpp"

using namespace std;
using namespace nv::shmem;

shared_ptr<SHMSensorAggregator> AggregationService::sensorAggregator = nullptr;

bool AggregationService::namespaceInit(string processName)
{
    lg2::info("Initializing shm namespace for process: {PROCESS_NAME}",
              "PROCESS_NAME", processName);
    try
    {
        ConfigReader::loadSHMMappingConfig();
    }
    catch (const exception& e)
    {
        lg2::error(
            "SHMEMDEBUG: Exception {EXCEPTION} while loading shm mapping config. ",
            "EXCEPTION", e.what());
        return false;
    }
    try
    {
        ConfigReader::loadNamespaceConfig();
        try
        {
            const auto& nameSpaceCfg =
                ConfigReader::getNameSpaceConfiguration();
            AggregationService::sensorAggregator =
                make_unique<SHMSensorAggregator>(move(processName),
                                                 move(nameSpaceCfg));
        }
        catch (const exception& e)
        {
            lg2::error(
                "SHMEMDEBUG: Exception {EXCEPTION} while reading namespace config. ",
                "EXCEPTION", e.what());
            return false;
        }
    }
    catch (const exception& e)
    {
        lg2::error(
            "SHMEMDEBUG: Exception {EXCEPTION} while loading namespace config. ",
            "EXCEPTION", e.what());
        return false;
    }
    return true;
}

bool AggregationService::updateTelemetry(const string& devicePath,
                                         const string& interface,
                                         const string& propName,
                                         DbusVariantType& value,
                                         const uint64_t timestamp, int rc,
                                         const string associatedEntityPath)
{
    if (sensorAggregator == nullptr)
    {
        return false;
    }
    if (rc != 0 && timestamp != 0)
    {
        SHMDEBUG("SHMEMDEBUG: Updating NAN value for key "
                 "{DEVICE_PATH}:{INTERFACE}:{PROPNAME}",
                 "DEVICE_PATH", devicePath, "INTERFACE", interface, "PROPNAME",
                 propName);
        return sensorAggregator->updateNanValue(devicePath, interface, propName,
                                                timestamp);
    }
    else
    {
        SHMDEBUG("SHMEMDEBUG: Updating Object for key "
                 "{DEVICE_PATH}:{INTERFACE}:{PROPNAME}",
                 "DEVICE_PATH", devicePath, "INTERFACE", interface, "PROPNAME",
                 propName);
        return sensorAggregator->updateSHMObject(devicePath, interface,
                                                 propName, value, timestamp,
                                                 associatedEntityPath);
    }
}
