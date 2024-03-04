/**
 * Copyright (c) 2023,2024 NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
#pragma once

#include <string>
#include <vector>

/*
 * @brief  The init API is used by the library to pre-populate all the meta data
 * necessary from data files and setup the data structures once on start up.
 * @return If init successfully it will return true else false
 */

bool smbusSlaveInit();

/*
 * @brief The update API will be used by all the sensor producers to regularly
 * refresh the values exposed on smbus slave.
 * @para1 DevicePath is sensor dbus object path
 * @para2 Interface is sensor dbus interface
 * @para3 PropName is sensor proeprty
 * @para4 raw data of the sensor from protocols - SMBPBI/PLDM/NSM etc
 * @para5 dataLength is length for raw value
 * @para6 Timestamp is last refresh time
 * @para7 Rc is return code of the sensor API
 *
 * @return It return zero for success else error RC
 */
int updateSmbusTelemetry(const std::string& devicePath,
                         const std::string& interface,
                         const std::string& propName, void* data,
                         size_t dataLength, const uint64_t timestamp, int rc);
