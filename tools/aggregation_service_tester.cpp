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



#include <telemetry_mrd_producer.hpp>

#include <boost/algorithm/string.hpp>
#include <phosphor-logging/log.hpp>

#include <iostream>

using namespace phosphor::logging;
using namespace nv::sensor_aggregation;
using namespace nv::shmem;

int main() {
  if (AggregationService::namespaceInit("gpumgrd")) {
    std::cout << "AggregationService::namespaceInit success" << std::endl;
  } else {
    std::cout << "AggregationService::namespaceInit failed" << std::endl;
  }
  auto tmpDevicePath =
      "/xyz/openbmc_project/sensors/temperature/HGX_Chassis_0_HSC_0_Temp_0";
  auto interface = "xyz.openbmc_project.Sensor.Value";
  auto propertyName = "Value";
  auto tmpParentPath = "HGX_Chassis_0";
  DbusVariantType tmpValue = 19.0625;
  uint64_t timeStamp = 23140448;
  uint64_t startTime = static_cast<uint64_t>(
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count());

  // TEST: New object creation sensor
  if (AggregationService::updateTelemetry(tmpDevicePath, interface,
                                          propertyName, tmpValue, 0, 0,
                                          tmpParentPath)) {
    std::cout << "AggregationService::updateTelemetry success for new object"
              << std::endl;
  } else {
    std::cout << "AggregationService::updateTelemetry failed for new object"
              << std::endl;
  }

  uint64_t endTime = static_cast<uint64_t>(
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count());
  std::cout << "AggregationService: Time spent for new object creation -> "
            << endTime - startTime << std::endl;

  // TEST: Value and timestamp update
  for (size_t i = 0; i < 10; i++) {
    startTime = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count());
    tmpValue = 29.0625;
    if (AggregationService::updateTelemetry(tmpDevicePath, interface,
                                            propertyName, tmpValue, 0, 0,
                                            tmpParentPath)) {
      std::cout << "AggregationService::updateTelemetry value and timestamp "
                   "update success"
                << std::endl;
    } else {
      std::cout << "AggregationService::updateTelemetry value and timestamp "
                   "update failed"
                << std::endl;
    }
    endTime = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count());
    std::cout << "AggregationService: Time for value and timestamp update "
              << i + 1 << "-> " << endTime - startTime << std::endl;
  }

  // TEST: Timestamp update
  timeStamp = 23150448;
  if (AggregationService::updateTelemetry(tmpDevicePath, interface,
                                          propertyName, tmpValue, timeStamp, 0,
                                          tmpParentPath)) {
    std::cout << "AggregationService::updateTelemetry timestamp update success."
              << std::endl;
  } else {
    std::cout << "AggregationService::updateTelemetry timestamp update failed."
              << std::endl;
  }

  // TEST: not applicable key test
  startTime = static_cast<uint64_t>(
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count());
  tmpDevicePath =
      "/xyz/openbmc_project/sensors/voltage/HGX_GPU_SXM_1_Voltage_0";
  if (AggregationService::updateTelemetry(tmpDevicePath, interface,
                                          propertyName, tmpValue, timeStamp, 0,
                                          tmpParentPath)) {
    std::cout << "AggregationService::updateTelemetry not applicable key test "
                 "success."
              << std::endl;
  } else {
    std::cout
        << "AggregationService::updateTelemetry not applicable key test failed."
        << std::endl;
  }
  endTime = static_cast<uint64_t>(
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count());
  std::cout << "AggregationService: Time spent for no matching key lookup -> "
            << endTime - startTime << std::endl;

  // TEST: nan handling simple type
  tmpDevicePath = "/xyz/openbmc_project/inventory/system/processors/GPU_SXM_2";
  interface = "xyz.openbmc_project.State.ProcessorPerformance";
  propertyName = "PCIeRXBytes";
  tmpValue = 2363508374;
  if (AggregationService::updateTelemetry(
          tmpDevicePath, interface, propertyName, tmpValue, timeStamp, 0)) {
    std::cout << "DEBUG: AggregationService: PCIeRXBytes add success"
              << std::endl;
    timeStamp = 23150448;
    if (AggregationService::updateTelemetry(
            tmpDevicePath, interface, propertyName, tmpValue, timeStamp, -1)) {
      std::cout << "AggregationService: nan handling simple type success"
                << std::endl;
    } else {
      std::cout << "AggregationService: nan handling simple type failed"
                << std::endl;
    }
  } else {
    std::cout << "DEBUG: AggregationService: PCIeRXBytes add success"
              << std::endl;
  }

  // TEST: nan handling sensor resource
  interface = "xyz.openbmc_project.Sensor.Value";
  propertyName = "Value";
  timeStamp = 23150448;
  tmpDevicePath =
      "/xyz/openbmc_project/sensors/temperature/HGX_Chassis_0_HSC_0_Temp_0";
  if (AggregationService::updateTelemetry(tmpDevicePath, interface,
                                          propertyName, tmpValue, timeStamp, -1,
                                          tmpParentPath)) {
    std::cout << "AggregationService: nan handling sensor resource success"
              << std::endl;
  } else {
    std::cout << "AggregationService: nan handling sensor resource failed"
              << std::endl;
  }

  // TEST: nan handling array resource

  // TEST: array properties dynamic change handling
  interface = "com.nvidia.GPMMetrics";
  propertyName = "NVDecInstanceUtilizationPercent";
  tmpDevicePath = "/xyz/openbmc_project/inventory/system/processors/GPU_SXM_2";
  std::vector<double> testVector = {8, 0, 0, 0, 0, 0, 0, 0, 0};
  DbusVariantType tmpValue1 = testVector;
  if (AggregationService::updateTelemetry(
          tmpDevicePath, interface, propertyName, tmpValue1, timeStamp, 0)) {
    std::cout << "AggregationService: array properties dynamic change handling "
                 "test success"
              << std::endl;
  } else {
    std::cout << "AggregationService: array properties dynamic change handling "
                 "test failed"
              << std::endl;
  }

  // TEST: GPMMetrics
  propertyName = "DMMAUtilizationPercent";
  interface = "com.nvidia.GPMMetrics";
  tmpDevicePath = "/xyz/openbmc_project/inventory/system/processors/GPU_SXM_2";
  DbusVariantType tmpValue2 = 0;
  if (AggregationService::updateTelemetry(
          tmpDevicePath, interface, propertyName, tmpValue2, timeStamp, 0)) {
    std::cout << "AggregationService:  GPMMetrics test success" << std::endl;
  } else {
    std::cout << "AggregationService:  GPMMetrics test failed" << std::endl;
  }
  sleep(300);
  return 0;
}