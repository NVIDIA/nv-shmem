/**
 * Copyright (c) 2023,2024 NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include "smbus_telemetry_target_api.hpp"

#include "smbus_telemetry_update.hpp"

#include <cstring>
#include <iostream>

bool smbusTelemetryInit = false;

bool smbusSlaveInit()
{
    // Checking for slave device
    if (std::strcmp(smbus_telemetry_update::i2cSlaveSysfs, "/dev/null") == 0)
    {
        lg2::error("SMBus slave device not configured");
        return false;
    }

    int rc = smbus_telemetry_update::loadFromCSV(SMBUS_SLAVE_TELEMETRY_CONFIG_CSV);
    if (rc != 0)
    {
        lg2::error("Failed to load data from CSV");
        return false;
    }
    smbusTelemetryInit = true;
    return true;
}

int updateSmbusTelemetry(const std::string& devicePath,
                         const std::string& interface,
                         const std::string& propName, void* data,
                         size_t dataLength, const uint64_t timestamp, int rc)
{
    // smbusSlaveInit not success telemetry update call return gracefully
    if (!smbusTelemetryInit)
        return 0;

    if (data != nullptr)
    {
        std::vector<uint8_t> value((uint8_t*)data, (uint8_t*)data + dataLength);
        int retVal = smbus_telemetry_update::smbusSlaveUpdate(
        devicePath, interface, propName, value, timestamp, rc);
        if (retVal != 0)
        {
            lg2::info("Failed to update smbus device {DEVICEPATH}", "DEVICEPATH",
                    devicePath);
            return retVal;
        }
    }
    return 0;
}
