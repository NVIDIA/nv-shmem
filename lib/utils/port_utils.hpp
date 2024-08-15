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
#include <string>
#include <unordered_map>

using namespace std;

namespace nv
{
namespace sensor_aggregation
{
namespace metricUtils
{

/* Map for link status pdi to redfish string */
static unordered_map<string, string> linkStatusTypeMap = {
    {"xyz.openbmc_project.Inventory.Decorator.PortState.LinkStatusType.LinkDown",
     "LinkDown"},
    {"xyz.openbmc_project.Inventory.Decorator.PortState.LinkStatusType.LinkUp",
     "LinkUp"},
    {"xyz.openbmc_project.Inventory.Decorator.PortState.LinkStatusType.NoLink",
     "NoLink"},
    {"xyz.openbmc_project.Inventory.Decorator.PortState.LinkStatusType.Starting",
     "Starting"},
    {"xyz.openbmc_project.Inventory.Decorator.PortState.LinkStatusType.Training",
     "Training"}};

/* Map for link state pdi to redfish string */
static unordered_map<string, string> linkStateTypeMap = {
    {"xyz.openbmc_project.Inventory.Decorator.PortState.LinkStates.Disabled",
     "Disabled"},
    {"xyz.openbmc_project.Inventory.Decorator.PortState.LinkStates.Enabled",
     "Enabled"},
    {"xyz.openbmc_project.Inventory.Decorator.PortState.LinkStates.Error",
     "Error"},
    {"xyz.openbmc_project.Inventory.Decorator.PortState.LinkStates.Unknown",
     "Unknown"}};

/* Map for PowerSystemInputType pdi to redfish string */
static unordered_map<string, string> powerSystemInputTypeTypeMap = {
    {"xyz.openbmc_project.State.Decorator.PowerSystemInputs.Status.Good",
     "Normal"},
    {"xyz.openbmc_project.State.Decorator.PowerSystemInputs.Status.Fault",
     "Fault"},
    {"xyz.openbmc_project.State.Decorator.PowerSystemInputs.Status.InputOutOfRange",
     "OutOfRange"},
    {"xyz.openbmc_project.State.Decorator.PowerSystemInputs.Status.Unknown",
     "Unknown"}};

/**
 * @brief Method to Get the Link Status Type redfish string from PDI name.
 *
 * @param[in] linkStatusType
 * @return string
 */
inline string getLinkStatusType(const string& linkStatusType)
{
    if (linkStatusTypeMap.find(linkStatusType) != linkStatusTypeMap.end())
    {
        return linkStatusTypeMap[linkStatusType];
    }
    return "";
}

/**
 * @brief Method to Get the Link State Type redfish string from PDI name.
 *
 * @param[in] linkStateType
 * @return string
 */
inline string getLinkStateType(const string& linkStateType)
{
    if (linkStateTypeMap.find(linkStateType) != linkStateTypeMap.end())
    {
        return linkStateTypeMap[linkStateType];
    }
    return "";
}

/**
 * @brief Method to Get the Link powerSystemInputType redfish string from PDI
 * name.
 *
 * @param[in] powerSystemInputType
 * @return string
 */
inline string getPowerSystemInputType(const string& powerSystemInputType)
{
    if (powerSystemInputTypeTypeMap.find(powerSystemInputType) !=
        powerSystemInputTypeTypeMap.end())
    {
        return powerSystemInputTypeTypeMap[powerSystemInputType];
    }
    return "";
}

} // namespace metricUtils
} // namespace sensor_aggregation
} // namespace nv
