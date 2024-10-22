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

#include "impl/shmem_map.hpp"

#include <chrono>
#include <iomanip>
#include <ios>
#include <iostream>

static constexpr auto now = std::chrono::steady_clock::now;
static const auto start = now();
using namespace std::chrono_literals;

void trace([[maybe_unused]] const auto&... msg)
{
    std::cerr << "at " << std::setw(8) << (now() - start) / 1ms << "ms :"
              << " ";
    (std::cerr << ... << msg) << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0]
                  << " [read|erase|perf|create|stat|readraw] [namespace]"
                  << std::endl;
        return 1; // Return an error code
    }

    try
    {
        auto name_space = argv[2];
        if (std::string(argv[1]) == "read")
        {
            nv::shmem::Map<nv::shmem::SensorMap, nv::shmem::SensorValue> mShmem(
                name_space, O_RDONLY);
            trace(name_space, "Shmem Created (read-only).");
            auto freeSize = mShmem.getFreeSize();
            trace(name_space, "Shmem FreeSize: ", freeSize, " Bytes");
            auto values = mShmem.getAllValues();
            for (auto e : values)
            {
                trace("Sensor ", e.metricProperty, " : ", e.timestampStr, " : ",
                      e.timestamp, " : ", e.sensorValue);
            }
        }
        else if (std::string(argv[1]) == "readraw")
        {
            nv::shmem::Map<nv::shmem::SensorMap, nv::shmem::SensorValue> mShmem(
                name_space, O_RDONLY);
            trace(name_space, "Shmem Created (read-only).");
            auto freeSize = mShmem.getFreeSize();
            trace(name_space, "Shmem FreeSize: ", freeSize, " Bytes");
            auto values = mShmem.getAllKeyValuePair();
            for (auto e : values)
            {
                trace("Object Key ", e.first, " : ", e.second);
            }
        }
        else if (std::string(argv[1]) == "erase")
        {
            nv::shmem::Map<nv::shmem::SensorMap, nv::shmem::SensorValue> mShmem(
                name_space, O_CREAT, 1024 * 1000);
            trace(name_space, "Shmem Created.");
            mShmem.clear();
            trace(name_space, "Shmem Erase done.");
        }
        else if (std::string(argv[1]) == "stat")
        {
            nv::shmem::Map<nv::shmem::SensorMap, nv::shmem::SensorValue> mShmem(
                name_space, O_RDONLY);
            trace(name_space, "Shmem Created (read-only).");
            while (1)
            {
                auto values = mShmem.getAllValues();
                for (auto e : values)
                {
                    trace("Sensor ", e.metricProperty, " : ", e.timestampStr,
                          " : ", e.timestamp, " : ", e.sensorValue);
                }
                usleep(1000000);
            }
        }
        else if (std::string(argv[1]) == "perf")
        {
            nv::shmem::Map<nv::shmem::SensorMap, nv::shmem::SensorValue> mShmem(
                name_space, O_CREAT, 1024 * 1000);
            trace(name_space, "Shmem Created.");

            for (int i = 0; i < 5000; i++)
            {
                int randomNumber = rand() % 5000 + 1;
                auto sensorName = std::string("HGX_Chassis_0_My_Sensor_" +
                                              std::to_string(randomNumber));
                nv::shmem::SensorValue value(
                    std::to_string(randomNumber),
                    "/redfish/v1/HGX_Chassis_0/Sensors/Sensor_" +
                        std::to_string(randomNumber),
                    0, "1/1/2022");
                mShmem.insert(sensorName, value);
            }
            trace("Objects Inserted.");

            while (1)
            {
                int randomNumber = rand() % 5000 + 1;
                auto sensorName = std::string("HGX_Chassis_0_My_Sensor_" +
                                              std::to_string(randomNumber));
                nv::shmem::SensorValue value;
                trace("search started--------------");
                if (mShmem.getValue(sensorName, value))
                {
                    trace("Key found.", value.sensorValue);
                }
                else
                {
                    trace("Key not found.", sensorName);
                }

                trace("update started--------------");
                mShmem.insert(sensorName, value);
                trace("update done--------------");

                usleep(10000);
            }
        }
        else if (std::string(argv[1]) == "create")
        {
            nv::shmem::Map<nv::shmem::SensorMap, nv::shmem::SensorValue> mShmem(
                name_space, O_CREAT, 1024 * 1000);
            trace(name_space, "Shmem Created.");
            for (int i = 0; i < 1000; i++)
            {
                int randomNumber = rand() % 1000 + 1;
                auto sensorName = std::string("HGX_Chassis_0_My_Sensor_" +
                                              std::to_string(randomNumber));
                const uint64_t now = static_cast<uint64_t>(
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now().time_since_epoch())
                        .count());
                nv::shmem::SensorValue value(
                    std::to_string(randomNumber),
                    "/redfish/v1/HGX_Chassis_0/Sensors/Sensor_" +
                        std::to_string(randomNumber),
                    now, "1/1/2022");
                mShmem.insert(sensorName, value);
            }
            trace("Objects Inserted.");
            // wait for other processes to read the data
            while (1)
            {
                sleep(100);
            }
        }
        else
        {
            std::cerr << "Usage: " << argv[0]
                      << " [read|erase|perf|create|stat|readraw] [namespace]"
                      << std::endl;
            return 1; // Return an error code
        }
    }
    catch (const std::exception& e)
    {
        trace("Exception: ", std::quoted(e.what()));
    }
}
