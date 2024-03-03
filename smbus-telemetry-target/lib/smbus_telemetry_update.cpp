/**
 * Copyright (c) 2023,2024 NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include "smbus_telemetry_update.hpp"
#include "error.hpp"

namespace smbus_telemetry_update
{

smbusSensorMap sensorDataMap;
uint64_t slaveI2cStaleThresholdMs;
int smbusSlaveVer;

bool isValidCSVData(std::vector<std::vector<std::string>>& csvData)
{
    return(csvData[0][0] != "slave_version" || csvData[1][0] != "staleness_threshold"
        || csvData[2][0] != "offset" || csvData[2][1] != "length" 
        || csvData[2][2] != "data_format" || csvData[2][3] != "dbus_objectpath" 
        || csvData[2][4] != "dbus_interface" || csvData[2][5] != "dbus_property" 
        || csvData[2][6] != "stale_offset" || csvData[2][7] != "stale_bit") ? false:true;
}

int loadFromCSV(const std::string& filename)
{
    try
    {
        std::ifstream file(filename.c_str());

        std::string line;
        std::string str;
        std::vector<std::vector<std::string>> smbusDetails;
        if (file.is_open())
        {
            while (std::getline(file, line))
            {
                std::vector<std::string> sensorToken;
                std::stringstream data_stream(line);
                while (getline(data_stream, str, ','))
                {
                    if (!str.empty())
                    {
                        sensorToken.push_back(str);
                    }
                }
                smbusDetails.push_back(sensorToken);
            }
            file.close();
        }
        else
        {
            lg2::error(
                "SMBus Slave Telemetry Config CSV file not found: {FILENAME}",
                "FILENAME", filename);
            return ErrorCode::ConfigFileNotFound;
        }

        if (!isValidCSVData(smbusDetails))
        {
            lg2::error("Invalid record on csv file");
            return ErrorCode::InvalidConfigData;
        }

        int smbusSensorDataCount = 0;
        for (auto& val : smbusDetails)
        {
            std::string keyValue = val[0].c_str();

            if (!keyValue.compare("slave_version"))
            {
                smbusSlaveVer = std::stoi(val[1], nullptr, 16);
                continue;
            }
            else if (!keyValue.compare("staleness_threshold"))
            {
                slaveI2cStaleThresholdMs = std::stoi(val[1]);
                continue;
            }
            else if (!keyValue.compare("offset"))
            {
                continue;
            }
            else
            {
                if(val.size() != SMBUS_DATA_RECORD_SIZE)
                {
                    lg2::error("Invalid smbus sensor data: {RECORDSIZE}",
                    "RECORDSIZE", val.size());
                    return ErrorCode::InvalidConfigData;
                }
                SmbusSensorData smbusSensorData;

                smbusSensorData.setSensorOffset(static_cast<uint16_t>(std::stoi(val[0], nullptr, 16)));
                smbusSensorData.setOffsetDataLength(stoi(val[1]));
                smbusSensorData.setDbusObjPath(val[3]);
                smbusSensorData.setDbusIface(val[4]);
                smbusSensorData.setDbusProperty(val[5]);

                if (val[6] != "NA")
                {
                    smbusSensorData.setStaleOffset(static_cast<int>(std::stoi(val[6], nullptr, 16)));
                }
                else
                {
                    smbusSensorData.setStaleOffset(-1);
                }

                if (val[7] != "NA")
                {
                    smbusSensorData.setStaleBit(stoi(val[7]));
                }
                else
                {
                    smbusSensorData.setStaleBit(-1);
                }
                std::string keyMap = smbusSensorData.getDbusObjPath() + "_" +
                                     smbusSensorData.getDbusIface() + "_" +
                                     smbusSensorData.getDbusProperty();
                sensorDataMap.insert(
                    std::pair<std::string, class SmbusSensorData>(
                        keyMap, smbusSensorData));
                smbusSensorDataCount++;
            }
        }
        lg2::info("Total smbus sensor records configured: {COUNT}",
                "COUNT", smbusSensorDataCount);
    }
    catch (const std::exception& e)
    {
        lg2::error("SMBus slave telemetry init failed {EXCEPTION}", "EXCEPTION",
                   e.what());
        return ErrorCode::FaildToLoadCSV;
    }

    return 0;
}

int smbusSlaveUpdate(std::string dbusObjPath, std::string iface,
                     std::string propName, std::vector<uint8_t> value,
                     uint64_t timestamp, int rc)
{
    std::string key = dbusObjPath + "_" + iface + "_" + propName;
    
    if (sensorDataMap.find(key) == sensorDataMap.end())
    {
	// SmbusUpdate sensor configuration not supported.
        return 0;
    }

    std::fstream eepromFile(i2cSlaveSysfs);
    if (!eepromFile)
    {
        lg2::error(
            "SMBus slave telemetry eeprom file not found : {I2CSLAVESYSFS}",
            "I2CSLAVESYSFS", i2cSlaveSysfs);
        return ErrorCode::SMBusSysfsPathNotFound;
    }
    // To avoid stale value on init
    if (sensorDataMap[key].initTs == true)
    {
        sensorDataMap[key].previousTimeStamp = timestamp;
        sensorDataMap[key].initTs = false;
    }
    // Refresh timestamp for checking staleness
    uint8_t stale = ((timestamp - sensorDataMap[key].previousTimeStamp) >
                     slaveI2cStaleThresholdMs)
                        ? 1
                        : 0;
    sensorDataMap[key].previousTimeStamp = timestamp;

    bool success = (rc == 0) // smbpbi rc
                       ? 1
                       : 0;
    if (!success)
    {
        stale = 1;
        value.assign(sensorDataMap[key].getOffsetDataLength(), 0xFF);
    }

    // Commit Value to the right offset
    try
    {
        std::vector<uint8_t>& valBytes = value;
        eepromFile.seekp(sensorDataMap[key].getSensorOffset());
        eepromFile.write(reinterpret_cast<const char*>(valBytes.data()),
                         std::min((uint8_t)sensorDataMap[key].getOffsetDataLength(),
                                  // commit min of actial bytes or mapped bytes
                                  static_cast<uint8_t>(valBytes.size())));
    }
    catch (const std::exception& e)
    {
        lg2::error("Unable to write data to eeprom file : {EXCEPTION}",
                   "EXCEPTION", e.what());
    }

    // read-modify-write the stale bit
    try
    {
        // Avoid stale offset update if staleness details configure as NA
        if (sensorDataMap[key].getStaleOffset() != -1 || sensorDataMap[key].getStaleBit() != -1)
        {
            // read
            uint8_t existingStaleValue = 0;
            eepromFile.seekp(sensorDataMap[key].getStaleOffset());
            eepromFile.read(reinterpret_cast<char*>(&existingStaleValue),
                            sizeof(existingStaleValue));

            // modify
            if (stale)
            {
                existingStaleValue |= (1 << sensorDataMap[key].getStaleBit());
            }
            else
            {
                existingStaleValue &= ~(1 << sensorDataMap[key].getStaleBit());
            }

            // write
            eepromFile.seekp(sensorDataMap[key].getStaleOffset());
            eepromFile.write(reinterpret_cast<char*>(&existingStaleValue),
                            sizeof(existingStaleValue));
        }
    }
    catch (const std::exception& e)
    {
        lg2::error("Unable to write stale to eeprom file :{EXCEPTION}",
                   "EXCEPTION", e.what());
    }
    return 0;
}

} // namespace smbus_telemetry_update
