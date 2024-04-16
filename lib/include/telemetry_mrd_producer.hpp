/*
 * SPDX-FileCopyrightText: Copyright (c) 2023-2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
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

/*
Sensor Producers will use the APIs defined in this header to update the
telemetry objects. APIs provided in this header will be used for

- Initialize the producer name
- Create Sensor Objects in the shared memory
- Update Value and/OR Timestamp values

Below are some examples on API usage, refer API documentation section in this
file for more details.

Init namespace:
*******************************************************************************
    This API is to initialize the namespace. This API takes the process name as
    input which will be the suffix for shared memory namespaces. This API
    should be called once in telemetry producers, before creating new telemetry
    objects.

Example:
-------------------------------------------------------------------------------
    std::string producerName = "gpumgrd";
    AggregationService::namespaceInit(producerName);

Update telemetry:
*******************************************************************************
    API to add new telemetry object, update existing telemetry object value and
    update nan. This API should be called by telemetry producers.
    All PlatformEnvironmentMetrics will be under /xyz/openbmc_project/sensors.
    This API expects associationPath for sensors paths and returns error if
    associationPath is not passed for the sensor objects.

Example: Insert
-------------------------------------------------------------------------------
    #include <telemetry_mrd_service.hpp>
    using namespace nv::sensor_aggregation;

    std::string devicePath =
        "/xyz/openbmc_project/sensors/temperature/HGX_Chassis_0_HSC_0_Temp_0";
    std::string interface = "xyz.openbmc_project.Sensor.Value";
    std::string propertyName = "Value";
    std::string parentPath = "HGX_Chassis_0";
    DbusVariantType value = 19.0625;
    uint64_t timeStamp = 23140448;

    if (AggregationService::updateTelemetry(devicePath, interface,
                                            propertyName, value, 0, 0,
                                            parentPath)) {
      std::cout << "AggregationService::updateTelemetry success for new object"
                << std::endl;
    } else {
      std::cout << "AggregationService::updateTelemetry failed for new object"
                << std::endl;
    }

Example: Update timestamp and value
-------------------------------------------------------------------------------
    value = 29.0625;
    timeStamp = 23150448
    if (AggregationService::updateTelemetry(devicePath, interface,
                                            propertyName, value, 0, 0,
                                            parentPath)) {
      std::cout << "AggregationService::updateTelemetry value and timestamp "
                    "update success"
                << std::endl;
    } else {
      std::cout << "AggregationService::updateTelemetry value and timestamp "
                    "update failed"
                << std::endl;
    }

Example: Update nan
-------------------------------------------------------------------------------
    if (AggregationService::updateTelemetry(
            devicePath, interface, propertyName, value, timeStamp, -1)) {
      std::cout << "AggregationService: nan handling simple type success"
                << std::endl;
    } else {
      std::cout << "AggregationService: nan handling simple type failed"
                << std::endl;
    }
*/

#pragma once
#include <shm_common.h>

namespace nv {
namespace sensor_aggregation {
class SHMSensorAggregator;
}
} // namespace nv

// using namespace std;
using namespace nv::sensor_aggregation;

namespace nv {

namespace shmem {

struct AggregationService {
private:
  static std::shared_ptr<SHMSensorAggregator> sensorAggregator;

public:
  /**
   * @brief API to initialize the namespace. This API takes the process name as
   * input which will be the suffix for shared memory namespaces. This API
   * should be called once in telemetry producers, before creating new telemetry
   * objects.
   *
   * @param[in] processName - process name of producer service
   * @return true
   * @return false
   */
  static bool namespaceInit(std::string processName);

  /**
   * @brief API to add new telemetry object, update existing telemetry object
   * value and update nan. This API should be called by telemetry producers.
   *
   * @param[in] devicePath - Device path of telemetry object.
   * @param[in] interface - Phosphor D-Bus interface of telemetry object.
   * @param[in] propName - Metric name.
   * @param[in] value - Metric value.
   * @param[in] timestamp - Timestamp of telemetry object.
   * @param[in] rc - Set this value to non zero for nan update.
   * @param[in] associatedEntityPath - optional for other metrics. Required for
   * platform environment metrics.
   * @return true
   * @return false
   */
  static bool updateTelemetry(const std::string &devicePath,
                              const std::string &interface,
                              const std::string &propName,
                              DbusVariantType &value, const uint64_t timestamp,
                              int rc,
                              const std::string associatedEntityPath = {});
};
} // namespace shmem
} // namespace nv