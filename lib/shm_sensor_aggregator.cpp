#include "impl/shm_sensor_aggregator.hpp"
#include "config.h"
#include <utils/time_utils.hpp>

#include <boost/algorithm/string.hpp>

using namespace std;
using namespace nv::sensor_aggregation;
using namespace nv::sensor_aggregation::metricUtils;

size_t SHMSensorAggregator::getMatchCount(string objPathKeyword,
                                          vector<string> &devicePathKeys) {
  vector<string> objectPathKeys;
  boost::trim_if(objPathKeyword, boost::is_any_of("/"));
  boost::split(objectPathKeys, objPathKeyword, boost::is_any_of("/"));
  size_t tmpMatchCount = 0;
  size_t devicePathStartIndex = 0;
  for (size_t objPathIndex = 0; objPathIndex < objectPathKeys.size();
       objPathIndex++) {
    for (size_t devicePathIndex = devicePathStartIndex;
         devicePathIndex < devicePathKeys.size(); devicePathIndex++) {
      if (objectPathKeys[objPathIndex] == devicePathKeys[devicePathIndex]) {
        devicePathStartIndex = devicePathIndex;
        tmpMatchCount += 1;
        break;
      }
    }
    // no match found for objectPath, not required to continue
    if (tmpMatchCount != objPathIndex + 1) {
      tmpMatchCount = 0;
      break;
    }
  }
  return tmpMatchCount;
}

MatchingNameSpaces SHMSensorAggregator::parseDevicePath(
    const sdbusplus::message::object_path &devicePathObj) {
  vector<tuple<SensorNameSpace, DeviceName, SubDeviceName, size_t>>
      matchingNameSpaces;
  string devicePath(devicePathObj);
  vector<string> devicePathKeys;
  DeviceName deviceName;
  SubDeviceName subDeviceName;
  size_t maxMatchCount = 0;
  boost::trim_if(devicePath, boost::is_any_of("/"));
  boost::split(devicePathKeys, devicePath, boost::is_any_of("/"));
  for (const auto &nameSpaceValues : nameSpaceConfig) {
    size_t tmpIndex = 0;
    for (const auto &nameSpaceValue : nameSpaceValues.second) {
      const auto &nameSpace = nameSpaceValues.first;
      auto currentMatchCount =
          getMatchCount(nameSpaceValue.first, devicePathKeys);
      if (currentMatchCount == 0) {
        continue;
      } else if (currentMatchCount > maxMatchCount) {
        maxMatchCount = currentMatchCount;
        if (maxMatchCount == 1) {
          deviceName = string(devicePathObj.filename());
        } else {
          if (devicePath.find("xyz/openbmc_project/sensors") == 0) {
            subDeviceName = string(devicePathObj.filename());
            deviceName = "";
          } else {
            deviceName =
                string(devicePathObj.parent_path().parent_path().filename());
            subDeviceName = string(devicePathObj.filename());
          }
        }
        matchingNameSpaces.clear();
        matchingNameSpaces.emplace_back(
            make_tuple(nameSpace, deviceName, subDeviceName, tmpIndex));
      } else if (currentMatchCount == maxMatchCount) {
        matchingNameSpaces.emplace_back(
            make_tuple(nameSpace, deviceName, subDeviceName, tmpIndex));
      }
      tmpIndex += 1;
    }
  }
  return matchingNameSpaces;
}

bool SHMSensorAggregator::insertShmemObject(
    const NameSpaceFields &nameSpaceFields, const string &sensorKey,
    const string &devicePath, const string &propName, const string &ifaceName,
    DbusVariantType &value, const uint64_t timestamp) {
  bool status = true;
  const uint64_t systemTimestamp =
      static_cast<uint64_t>(chrono::duration_cast<chrono::milliseconds>(
                                chrono::system_clock::now().time_since_epoch())
                                .count()) -
      static_cast<uint64_t>(chrono::duration_cast<chrono::milliseconds>(
                                chrono::steady_clock::now().time_since_epoch())
                                .count()) +
      timestamp;

  string timeStampStr =
      nv::sensor_aggregation::metricUtils::getDateTimeUintMs(systemTimestamp);
  auto shmNamespace = producerName + "_" + PLATFORMDEVICEPREFIX +
                      nameSpaceFields.sensorNameSpace + "_0";
  if (!sensorMapIntf.isNameSpacePresent(shmNamespace)) {
    const size_t shmSize = 1024 * 1000;
    if (!sensorMapIntf.createNamespace(shmNamespace, shmSize)) {
      status = false;
      return status;
    }
  }
  {
    scoped_lock lock(nameSpaceMapLock);
    nameSpaceMap.emplace(sensorKey, nameSpaceFields);
  }
  auto [metricValues, isList] = getMetricValues(
      nameSpaceFields.sensorNameSpace, nameSpaceFields.deviceName,
      nameSpaceFields.subDeviceName, devicePath, propName, ifaceName, value);
  if (isList) {
    scoped_lock lock(nameSpaceMapLock);
    nameSpaceMap[sensorKey].arraySize = metricValues.size();
  }
  for (const auto &metricVal : metricValues) {
    if (!metricVal.first.empty()) {
      auto [tmpMetricProp, tmpMetricVal] = metricVal.second;
      SensorValue sensorValue(tmpMetricVal, tmpMetricProp, timestamp,
                              timeStampStr);
      if (!sensorMapIntf.insert(shmNamespace, metricVal.first, sensorValue)) {
        status = false;
      }
    }
  }
  return status;
}

inline string SHMSensorAggregator::getSensorMapKey(const string &devicePath,
                                                   const string &interface,
                                                   const string &propName) {
  return interface + "." + devicePath + "." + propName;
}

bool SHMSensorAggregator::updateNanValue(
    const string &devicePath, const string &interface, const string &propName,
    [[maybe_unused]] const uint64_t timestamp) {
  bool status = true;
  const uint64_t systemTimestamp =
      static_cast<uint64_t>(chrono::duration_cast<chrono::milliseconds>(
                                chrono::system_clock::now().time_since_epoch())
                                .count()) -
      static_cast<uint64_t>(chrono::duration_cast<chrono::milliseconds>(
                                chrono::steady_clock::now().time_since_epoch())
                                .count()) +
      timestamp;

  string timeStampStr =
      nv::sensor_aggregation::metricUtils::getDateTimeUintMs(systemTimestamp);
  auto sensorKey = getSensorMapKey(devicePath, interface, propName);
  if (nameSpaceMap.find(sensorKey) == nameSpaceMap.end() &&
      notApplicableKeys.find(sensorKey) == notApplicableKeys.end()) {
    return status;
  }
  const auto [nameSpace, deviceName, subDeviceName, arraySize] =
      nameSpaceMap[sensorKey];
  string shmNamespace =
      producerName + "_" + PLATFORMDEVICEPREFIX + nameSpace + "_0";

  // clear keys for all array data preserve first index to update nan
  if (arraySize > 1) {
    for (size_t i = 1; i < arraySize; i++) {
      string shmKey = sensorKey + "/" + to_string(i);
      if (!sensorMapIntf.erase(shmNamespace, shmKey)) {
        status = false;
      }
    }
    auto shmKey = sensorKey + "/0";
    if (!sensorMapIntf.updateValueAndTimeStamp(shmNamespace, shmKey, "nan",
                                               timestamp, timeStampStr)) {

      lg2::error("SHMEMDEBUG: update timestamp and value failed {SHMKEY}",
                 "SHMKEY", shmKey);
      status = false;
    }
  } else {
    if (!sensorMapIntf.updateValueAndTimeStamp(shmNamespace, sensorKey, "nan",
                                               timestamp, timeStampStr)) {
      lg2::error("SHMEMDEBUG: update timestamp and value failed {SHMKEY}",
                 "SHMKEY", sensorKey);
      status = false;
    }
  }
  return status;
}

bool SHMSensorAggregator::handleObjectInsertion(
    MatchingNameSpaces matchingNameSpaces, const string &devicePath,
    const string &interface, const string &propName, const string &sensorKey,
    DbusVariantType &value, const uint64_t timestamp,
    const string associatedEntityPath) {
  for (const auto &matchingNameSpace : matchingNameSpaces) {
    auto [nameSpace, deviceName, subDeviceName, nameSpaceMapIndex] =
        matchingNameSpace;
    if (nameSpace.empty()) {
      lg2::error("SHMEMDEBUG: No matching namespace found for device path "
                 "{DEVICE_PATH}",
                 "DEVICE_PATH", devicePath);
      scoped_lock lock(notApplicableKeysLock);
      notApplicableKeys.emplace(sensorKey, 1);
      return false;
    }
    if (deviceName.empty()) {
      if (associatedEntityPath.empty()) {
        lg2::error("SHMEMDEBUG: Parent path should not be empty for "
                   "sensor resource: {DEVICE_PATH}",
                   "DEVICE_PATH", devicePath);
        scoped_lock lock(notApplicableKeysLock);
        notApplicableKeys.emplace(sensorKey, 1);
        return false;
      }
      deviceName =
          associatedEntityPath.substr(associatedEntityPath.rfind("/") + 1);
    }
    const auto &propertyList =
        nameSpaceConfig[nameSpace][nameSpaceMapIndex].second;
    if (find(propertyList.begin(), propertyList.end(), propName) !=
        propertyList.end()) {
      NameSpaceFields nameSpaceFields = {nameSpace, deviceName, subDeviceName,
                                         0};
      return insertShmemObject(nameSpaceFields, sensorKey, devicePath, propName,
                               interface, value, timestamp);
    } else {
      scoped_lock lock(notApplicableKeysLock);
      notApplicableKeys.emplace(sensorKey, 1);
    }
  }
  return true;
}

bool SHMSensorAggregator::handleArrayUpdates(
    unordered_map<SHMKey, SHMValue> &metricValues, bool isList,
    const string &shmNamespace, const string &sensorKey,
    const uint64_t timestamp, const string &timeStampStr, size_t arraySize) {
  bool status = true;
  if (!isList) {
    for (const auto &metricVal : metricValues) {
      string propertyValue = get<1>(metricVal.second);
      if (!sensorMapIntf.updateValueAndTimeStamp(shmNamespace, sensorKey,
                                                 propertyValue, timestamp,
                                                 timeStampStr)) {
        status = false;
      }
    }
  } else if (arraySize <= metricValues.size()) {
    for (size_t i = arraySize - 1; i < metricValues.size() - 1; i++) {
      string shmKey = sensorKey + "/" + to_string(i);
      if (!sensorMapIntf.erase(shmNamespace, shmKey)) {
        status = false;
      }
    }
    size_t arrayCount = 0;
    for (const auto &metricVal : metricValues) {
      string shmKey = sensorKey + "/" + to_string(arrayCount);
      string propertyValue = get<1>(metricVal.second);
      if (!sensorMapIntf.updateValueAndTimeStamp(
              shmNamespace, shmKey, propertyValue, timestamp, timeStampStr)) {
        status = false;
      }
      arrayCount += 1;
    }
    {
      scoped_lock lock(nameSpaceMapLock);
      nameSpaceMap[sensorKey].arraySize = metricValues.size();
    }
  } else if (metricValues.size() > arraySize) {
    // case when new array elements gets added.
    size_t arrayCount = 0;
    for (const auto &metricVal : metricValues) {
      auto shmKey = sensorKey + "/" + to_string(arrayCount);
      auto [tmpMetricProp, tmpMetricVal] = metricVal.second;
      if (arrayCount < arraySize) {
        if (!sensorMapIntf.updateValueAndTimeStamp(
                shmNamespace, shmKey, tmpMetricVal, timestamp, timeStampStr)) {
          status = false;
        }
      } else {
        SensorValue sensorValue(tmpMetricVal, tmpMetricProp, timestamp,
                                timeStampStr);
        if (!sensorMapIntf.insert(shmNamespace, tmpMetricVal, sensorValue)) {
          status = false;
        }
      }
      arrayCount += 1;
    }
  }
  return status;
}

bool SHMSensorAggregator::updateSHMObject(const string &devicePath,
                                          const string &interface,
                                          const string &propName,
                                          DbusVariantType &value,
                                          const uint64_t timestamp,
                                          const string associatedEntityPath) {
  auto sensorKey = getSensorMapKey(devicePath, interface, propName);
  bool status = true;
  if (nameSpaceMap.find(sensorKey) != nameSpaceMap.end()) {
    auto [nameSpace, deviceName, subDeviceName, arraySize] =
        nameSpaceMap[sensorKey];

    const uint64_t systemTimestamp =
        static_cast<uint64_t>(
            chrono::duration_cast<chrono::milliseconds>(
                chrono::system_clock::now().time_since_epoch())
                .count()) -
        static_cast<uint64_t>(
            chrono::duration_cast<chrono::milliseconds>(
                chrono::steady_clock::now().time_since_epoch())
                .count()) +
        timestamp;

    string timeStampStr =
        nv::sensor_aggregation::metricUtils::getDateTimeUintMs(systemTimestamp);

    string shmNamespace =
        producerName + "_" + PLATFORMDEVICEPREFIX + nameSpace + "_0";
    if (arraySize == 0) {
      auto metricVal = getMetricValue(propName, interface, value);
      string propertyValue = get<1>(metricVal);
      if (!sensorMapIntf.updateValueAndTimeStamp(shmNamespace, sensorKey,
                                                 propertyValue, timestamp,
                                                 timeStampStr)) {
        status = false;
      }
    } else {
      auto [metricValues, isList] =
          getMetricValues(nameSpace, deviceName, subDeviceName, devicePath,
                          propName, interface, value);
      status = handleArrayUpdates(metricValues, isList, shmNamespace, sensorKey,
                                  timestamp, timeStampStr, arraySize);
    }
    return status;
  }
  if (notApplicableKeys.find(sensorKey) != notApplicableKeys.end()) {
    return true; // sensor key not applicable
  }
  auto matchingNameSpaces = parseDevicePath(devicePath);
  if (matchingNameSpaces.size() == 0) {
    lg2::error("SHMEMDEBUG: No matching namespace found for device path "
               "{DEVICE_PATH}",
               "DEVICE_PATH", devicePath);
    scoped_lock lock(notApplicableKeysLock);
    notApplicableKeys.emplace(sensorKey, 1);
    return false;
  }
  status =
      handleObjectInsertion(matchingNameSpaces, devicePath, interface, propName,
                            sensorKey, value, timestamp, associatedEntityPath);
  return status;
}