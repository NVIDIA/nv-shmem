/**
 * Copyright (c) 2023,2024 NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
#include <phosphor-logging/lg2.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "config.h"

namespace smbus_telemetry_update
{
constexpr auto i2cSlaveSysfs = SMBUS_SYSFS_PATH;
using smbusSensorMap = std::map<std::string, class SmbusSensorData>;

/*
 * @brief  SmbusSensorData carries the smbus sensor data record
 *
 * length - Length of byte to write on eeprom device
 * staleBit - This value represent staleness detail for the sensor
 * staleOffset - This value represent offset to read/write sensor stale bit
 * previousTimeStamp - Timestamp of last refresh data
 * offset - This value represent offset to write sensor value
 * dbusObjPath - dbusOject path name of the sensor
 * dbusIface - dbusInterface name of the sensor
 * dbusProperty - dbusProeprty name which carries sensor value
 * initTs - Default initTs is true to avoid inital stale update
 */

class SmbusSensorData
{
  private:
    int length;
    int staleBit;
    int staleOffset;
    uint16_t offset;
    std::string dbusObjPath;
    std::string dbusIface;
    std::string dbusProperty;

  public:
    uint64_t previousTimeStamp;
    bool initTs;

    SmbusSensorData(): length(0), staleBit(0), staleOffset(0), offset(0),
        previousTimeStamp(0), initTs(true)
    {

    }
    ~SmbusSensorData()
    {

    }
    
    /* @brief Helper function to get smbus offset length */
    int getOffsetDataLength()
    {
        return this->length;
    }

    /* @brief Helper function to set smbus offset length */
    void setOffsetDataLength(int length)
    {
        this->length = length;
    }

    /* @brief Helper function to get smbus stale offset */
    int getStaleOffset()
    {
        return this->staleOffset;
    }

    /* @brief Helper function to set smbus stale offset */
    void setStaleOffset(int staleOffset)
    {
        this->staleOffset = staleOffset;
    }

    /* @brief Helper function to get smbus stale bit */
    int getStaleBit()
    {
        return this->staleBit;
    }

    /* @brief Helper function to set smbus stale bit */
    void setStaleBit(int staleBit)
    {
        this->staleBit = staleBit;
    }

    /* @brief Helper function to get smbus offset of the sensor */
    uint16_t getSensorOffset()
    {
        return this->offset;
    }

    /* @brief Helper function to set smbus offset of the sensor */
    void setSensorOffset(uint16_t offset)
    {
        this->offset = offset;
    }

    /* @brief Helper function to get dbus object path
              which have the sensor dbus iface */
    std::string getDbusObjPath()
    {
        return this->dbusObjPath;
    }

    /* @brief Helper function to set dbus object path
              which have the sensor dbus iface */
    void setDbusObjPath(std::string dbusObjPath)
    {
        this->dbusObjPath = dbusObjPath;
    }
    /* @brief Helper function to get dbus Interface
              which carries of the sensor property */
    std::string getDbusIface()
    {
        return this->dbusIface;
    }

    /* @brief Helper function to set dbus Interface
              which carries of the sensor property */
    void setDbusIface(std::string dbusIface)
    {
        this->dbusIface = dbusIface;
    }

    /* @brief Helper function to get dbus property name
              which carries the sensor value */
    std::string getDbusProperty()
    {
        return this->dbusProperty;
    }

    /* @brief Helper function to set dbus property name
              which carries the sensor value */
    void setDbusProperty(std::string dbusProperty)
    {
        this->dbusProperty = dbusProperty;
    }
};

/*
 * @brief  The loadFromCSV API is used to parse the data from csv file
 * and mapped the smbus record with its corresponding sensors
 * @para1 filename - csv file path
 * @return It return zero for success else error RC
 */

int loadFromCSV(const std::string& filename);

/*
 * @brief The smbusSlaveUpdate API will be used to update sensor data
 * on slave eeprom device on it corresponding offets.
 * @para1 dbusObjPath is sensor dbus object path
 * @para2 iface is sensor dbus interface
 * @para3 propName is sensor proeprty
 * @para4 value of the sensor from protocols - SMBPBI/PLDM/NSM etc
 * @para5 timestamp is last refresh time
 * @para6 rc is return code of the sensor API
 *
 * @return It return zero for success else error RC
 */
int smbusSlaveUpdate(std::string dbusObjPath, std::string iface,
                     std::string propName, std::vector<uint8_t> value,
                     uint64_t timestamp, int rc);

} // namespace smbus_telemetry_update