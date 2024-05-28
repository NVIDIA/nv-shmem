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
    {"xyz.openbmc_project.Inventory.Item.Port.LinkStatusType.LinkDown",
     "LinkDown"},
    {"xyz.openbmc_project.Inventory.Item.Port.LinkStatusType.LinkUp", "LinkUp"},
    {"xyz.openbmc_project.Inventory.Item.Port.LinkStatusType.NoLink", "NoLink"},
    {"xyz.openbmc_project.Inventory.Item.Port.LinkStatusType.Starting",
     "Starting"},
    {"xyz.openbmc_project.Inventory.Item.Port.LinkStatusType.Training",
     "Training"}};

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

} // namespace metricUtils
} // namespace sensor_aggregation
} // namespace nv
