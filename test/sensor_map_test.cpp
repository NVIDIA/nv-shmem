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

#include "impl/shmem_map.hpp"

#include <memory>

#include "gmock/gmock.h"
#include <gtest/gtest.h>

using namespace nv::shmem;

class SensorMapTests : public testing::Test
{
  public:
    SensorMapTests()
    {
        auto name_space = "maptest";
        mShmem = std::make_unique<Map<SensorMap, SensorValue>>(
            name_space, O_CREAT, 1024 * 1000);
    }

    ~SensorMapTests() {}

    static void TearDownTestCase() {}
    std::unique_ptr<Map<SensorMap, SensorValue>> mShmem;
};

TEST_F(SensorMapTests, testSensorMapCreate)
{
    for (int i = 0; i < 5; i++)
    {
        auto sensorName =
            std::string("HGX_Chassis_0_My_Sensor_" + std::to_string(i));
        nv::shmem::SensorValue value(
            std::to_string(i),
            "/redfish/v1/HGX_Chassis_0/Sensors/Sensor_" + std::to_string(i), 0,
            "1/1/2022");
        mShmem->insert(sensorName, value);
    }

    EXPECT_EQ(mShmem->size(), 5);
}

TEST_F(SensorMapTests, testSensorMapClear)
{
    for (int i = 0; i < 5; i++)
    {
        std::string sensorName =
            std::string("HGX_Chassis_0_My_Sensor_" + std::to_string(i));
        nv::shmem::SensorValue value(
            std::to_string(i),
            "/redfish/v1/HGX_Chassis_0/Sensors/Sensor_" + std::to_string(i), 0,
            "1/1/2022");
        mShmem->insert(sensorName, value);
    }
    EXPECT_NE(mShmem->size(), 0);

    mShmem->clear();
    EXPECT_EQ(mShmem->size(), 0);
}

TEST_F(SensorMapTests, testSensorMapGetValue)
{
    mShmem->clear();
    EXPECT_EQ(mShmem->size(), 0);

    auto sensorName = "HGX_Chassis_0_My_Sensor_1";
    nv::shmem::SensorValue value(std::to_string(100),
                                 "/redfish/v1/HGX_Chassis_0/Sensors/Sensor_1",
                                 1699255438, "1/1/2022");
    mShmem->insert(sensorName, value);
    nv::shmem::SensorValue readValue;
    EXPECT_TRUE(mShmem->getValue(sensorName, readValue));

    EXPECT_EQ(value.sensorValue, readValue.sensorValue);
    EXPECT_EQ(value.timestampStr, readValue.timestampStr);
    EXPECT_EQ(value.metricProperty, readValue.metricProperty);
    EXPECT_EQ(value.timestamp, readValue.timestamp);
}

TEST_F(SensorMapTests, testSensorMapGetAllValues)
{
    mShmem->clear();
    EXPECT_EQ(mShmem->size(), 0);

    auto sensorName = "HGX_Chassis_0_My_Sensor_100";
    nv::shmem::SensorValue value(std::to_string(100),
                                 "/redfish/v1/HGX_Chassis_0/Sensors/Sensor_1",
                                 1699255438, "1/1/2022");
    mShmem->insert(sensorName, value);

    for (int i = 0; i < 4; i++)
    {
        std::string sensorName =
            std::string("HGX_Chassis_0_My_Sensor_" + std::to_string(i));
        nv::shmem::SensorValue value(
            std::to_string(i),
            "/redfish/v1/HGX_Chassis_0/Sensors/Sensor_" + std::to_string(i), 0,
            "1/1/2022");
        mShmem->insert(sensorName, value);
    }

    auto allSensors = mShmem->getAllValues();
    EXPECT_EQ(allSensors.size(), 5);
    EXPECT_EQ(allSensors[2].sensorValue, value.sensorValue);
    EXPECT_EQ(allSensors[2].timestampStr, value.timestampStr);
    EXPECT_EQ(allSensors[2].metricProperty, value.metricProperty);
    EXPECT_EQ(allSensors[2].timestamp, value.timestamp);
}

TEST_F(SensorMapTests, testSensorMapErase)
{
    mShmem->clear();
    EXPECT_EQ(mShmem->size(), 0);

    for (int i = 0; i < 10; i++)
    {
        std::string sensorName =
            std::string("HGX_Chassis_0_My_Sensor_" + std::to_string(i));
        nv::shmem::SensorValue value(
            std::to_string(i),
            "/redfish/v1/HGX_Chassis_0/Sensors/Sensor_" + std::to_string(i), 0,
            "1/1/2022");
        mShmem->insert(sensorName, value);
    }

    auto sensorName = "HGX_Chassis_0_My_Sensor_1";
    nv::shmem::SensorValue value(std::to_string(100),
                                 "/redfish/v1/HGX_Chassis_0/Sensors/Sensor_1",
                                 1699255438, "1/1/2022");
    mShmem->insert(sensorName, value);

    EXPECT_TRUE(mShmem->getValue(sensorName, value));
    mShmem->erase(sensorName);
    EXPECT_FALSE(mShmem->getValue(sensorName, value));
}

TEST_F(SensorMapTests, testSensorMapUpdateValue)
{
    mShmem->clear();
    EXPECT_EQ(mShmem->size(), 0);

    auto sensorName = "HGX_Chassis_0_My_Sensor_1";
    nv::shmem::SensorValue value(std::to_string(100),
                                 "/redfish/v1/HGX_Chassis_0/Sensors/Sensor_1",
                                 1699255438, "1/1/2022");
    mShmem->insert(sensorName, value);

    mShmem->updateValue(sensorName, "101");

    nv::shmem::SensorValue readValue;
    EXPECT_TRUE(mShmem->getValue(sensorName, readValue));

    EXPECT_EQ("101", readValue.sensorValue);
    EXPECT_EQ(value.timestampStr, readValue.timestampStr);
    EXPECT_EQ(value.metricProperty, readValue.metricProperty);
    EXPECT_EQ(value.timestamp, readValue.timestamp);
}

TEST_F(SensorMapTests, testSensorMapUpdateTimestamp)
{
    mShmem->clear();
    EXPECT_EQ(mShmem->size(), 0);

    auto sensorName = "HGX_Chassis_0_My_Sensor_1";
    nv::shmem::SensorValue value(std::to_string(100),
                                 "/redfish/v1/HGX_Chassis_0/Sensors/Sensor_1",
                                 1699255438, "1/1/2022");
    mShmem->insert(sensorName, value);

    mShmem->updateTimestamp(sensorName, 1699255439, "1/2/2022");

    nv::shmem::SensorValue readValue;
    EXPECT_TRUE(mShmem->getValue(sensorName, readValue));

    EXPECT_EQ(value.sensorValue, readValue.sensorValue);
    EXPECT_EQ("1/2/2022", readValue.timestampStr);
    EXPECT_EQ(value.metricProperty, readValue.metricProperty);
    EXPECT_EQ(1699255439, readValue.timestamp);
}

TEST_F(SensorMapTests, testSensorMapReadOnlyMapErrorInUpdate)
{
    auto name_space = "maptest";
    auto mShmemROnly = std::make_unique<Map<SensorMap, SensorValue>>(name_space,
                                                                     O_RDONLY);
    auto sensorName = "HGX_Chassis_0_My_Sensor_1";
    nv::shmem::SensorValue value(std::to_string(100),
                                 "/redfish/v1/HGX_Chassis_0/Sensors/Sensor_1",
                                 1699255438, "1/1/2022");
    EXPECT_THROW(mShmemROnly->insert(sensorName, value), std::runtime_error);
}
