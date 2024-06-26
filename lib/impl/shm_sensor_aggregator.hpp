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

#include "impl/config_json_reader.hpp"
#include "shm_sensormap_intf.hpp"

#include <shm_common.h>

#include <phosphor-logging/lg2.hpp>
#include <sdbusplus/bus.hpp>
#include <utils/metric_report_utils.hpp>

#include <mutex>
#include <unordered_map>
#include <utility>

namespace nv
{
namespace sensor_aggregation
{
using namespace std;
using namespace nv::sensor_aggregation::metricUtils;
using DevicePath = string;
using DeviceName = string;
using SubDeviceName = string;
using SensorPath = string;
struct NameSpaceFields
{
    SensorNameSpace sensorNameSpace;
    DeviceName deviceName;
    SubDeviceName subDeviceName;
    size_t arraySize;
};
using NameSpaceMap = unordered_map<string, NameSpaceFields>;
using ConfigKeyLookup = unordered_map<string, string>;
using MatchingNameSpaces =
    vector<tuple<SensorNameSpace, DeviceName, SubDeviceName, size_t>>;

/**
 * @brief Sensor aggregator class provides the functionality of inserting new
 * object to shared memory, updating the value and timestamp and nan.
 *
 */
class SHMSensorAggregator
{
  public:
    SHMSensorAggregator() = delete;
    SHMSensorAggregator(const SHMSensorAggregator&) = delete;
    SHMSensorAggregator(SHMSensorAggregator&&) = delete;
    SHMSensorAggregator& operator=(const SHMSensorAggregator&) = delete;
    SHMSensorAggregator& operator=(SHMSensorAggregator&&) = delete;
    ~SHMSensorAggregator() = default;

    /**
     * @brief SHMSensorAggregator object
     *
     */
    explicit SHMSensorAggregator(string producerName,
                                 NameSpaceConfiguration nameSpaceCfg) :
        producerName(move(producerName)), nameSpaceConfig(move(nameSpaceCfg))
    {}

    /**
     * @brief This method inserts shared memory object if entry is not present
     * in map. If it's an existing object then value and timestamp will be
     * updated. For simple data types the values will be updated as is. Array
     * values are indexed and for value update it can be New elements insertion,
     * Removal of elements and updates or just updating values of existing
     * elements.
     *
     * @param[in] devicePath - Device path of telemetry object.
     * @param[in] interface - Phosphor D-Bus interface of telemetry object
     * @param[in] propName - Metric name.
     * @param[in] value - Metric value.
     * @param[in] timestamp - Timestamp of telemetry object.
     * @param[in] associatedEntityPath - optional for other metrics. Required
     * for platform environment metrics.
     * @return true
     * @return false
     */
    bool updateSHMObject(const string& devicePath, const string& interface,
                         const string& propName, DbusVariantType& value,
                         const uint64_t timestamp,
                         const string associatedEntityPath = {});

    /**
     * @brief This method updates nan value in shared memory. This API should be
     * called when rc is non zero. For arrays only one element will be retained
     * and remaining elements will be removed. Metric value will be changed to
     * 'nan'.
     *
     * @param devicePath
     * @param interface
     * @param propName
     * @param timestamp
     * @return true
     * @return false
     */
    bool updateNanValue(const string& devicePath, const string& interface,
                        const string& propName, const uint64_t timestamp);

    /**
     * @brief Method to create sensor namespace in shared memory.
     *
     * @return bool
     */
    bool createShmemNamespace();

  private:
    string producerName;
    mutex nameSpaceMapLock;
    mutex notApplicableKeysLock;
    NameSpaceConfiguration nameSpaceConfig;
    shmem::ShmSensorMapIntf sensorMapIntf;
    NameSpaceMap nameSpaceMap;
    unordered_map<string, uint8_t> notApplicableKeys;
    /**
     * @brief read configuration from json file and update intermediate data
     * structure for subsequent parsing.
     *
     */
    void readConfigJson();
    /**
     * @brief Get match count for device path passed from producer with device
     * path from config file.
     *
     * @param[in] objPathKeyword - object path keyword
     * @param[in] devicePathKeys - device path keys
     * @return size_t match count
     */
    size_t getMatchCount(string objPathKeyword, vector<string>& devicePathKeys);
    /**
     * @brief This method compares device path passed by producer with the paths
     * in config. For matching paths it returns corresponding namespaces along
     * with device and sub device name.
     *
     * @param[in] devicePathObj - device path passed by producer
     * @return MatchingNameSpaces
     */
    MatchingNameSpaces
        parseDevicePath(const sdbusplus::message::object_path& devicePathObj);

    /**
     * @brief Method to handle new shared memory object insertion
     *
     * @param matchingNameSpaces - namespace vector matching for the device path
     * @param devicePath - device path passed by producer
     * @param interface - pdi
     * @param propName - metric name
     * @param[in] sensorKey - sensor key
     * @param value - metric value
     * @param timestamp - timestamp int in epoch
     * @param associatedEntityPath - chassis association path
     * @return true
     * @return false
     */
    bool handleObjectInsertion(MatchingNameSpaces matchingNameSpaces,
                               const string& devicePath,
                               const string& interface, const string& propName,
                               const string& sensorKey, DbusVariantType& value,
                               const uint64_t timestamp,
                               const string associatedEntityPath);

    /**
     * @brief Method to handle shared memory array value updates. This method
     * handles array updates, removal and addition based on size.
     *
     * @param[in] metricValues - metric values
     * @param[in] isList - bool to indicate a list
     * @param[in] shmNamespace - shared memory namespace
     * @param[in] sensorKey - sensor key
     * @param[in] timestamp timestamp int in epoch
     * @param[in] timeStampStr - redfish timestamp string
     * @param[in] arraySize - existing array size in the map
     * @return true
     * @return false
     */
    bool handleArrayUpdates(unordered_map<SHMKey, SHMValue>& metricValues,
                            bool isList, const string& shmNamespace,
                            const string& sensorKey, const uint64_t timestamp,
                            const string& timeStampStr, size_t arraySize);

    /**
     * @brief Method to insert telemetry object to shared memory. This method is
     * responsible for timestamp formatting, shared memory initialization calls
     * and formatting the array values.
     *
     * @param[in] nameSpaceFields - namespace field parameters.
     * @param[in] sensorKey - sensor key value in shared memory
     * @param[in] devicePath - device path passed from producer
     * @param[in] propName - Metric name
     * @param[in] ifaceName - PDI name
     * @param[in] value - Metric value
     * @param[in] timestamp - timestamp in epoch
     * @return true
     * @return false
     */
    bool insertShmemObject(const NameSpaceFields& nameSpaceFields,
                           const string& sensorKey, const string& devicePath,
                           const string& propName, const string& ifaceName,
                           DbusVariantType& value, const uint64_t timestamp);

    /**
     * @brief Method to create sensor key value to update in shared memory.
     * Sensor key is created by adding device path, interface and metric name.
     *
     * @param[in] devicePath - device path passed from producer
     * @param[in] interface - PDI name
     * @param[in] propName - Metric name
     * @return string
     */
    inline string getSensorMapKey(const string& devicePath,
                                  const string& interface,
                                  const string& propName);
};
} // namespace sensor_aggregation
} // namespace nv
