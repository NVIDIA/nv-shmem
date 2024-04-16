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



#include "impl/shmem_map.hpp"

using namespace std;
using namespace nv::shmem;

template <>
Map<SensorMap, SensorValue>::Map(const string &nameSpace, const int opts,
                                 size_t maxSize)
    : ManagedShmem(nameSpace, opts, maxSize) {
  mapImpl = memory->find<SensorMap>(string(nameSpace + "map").c_str()).first;
  if (mapImpl == nullptr) {
    mapImpl = memory->construct<SensorMap>(string(nameSpace + "map").c_str())(
        less<nv::shmem::char_string_t>(), *voidAllocator);
    if (mapImpl == nullptr) {
      throw BadMapException();
    }
  }

  mapImpl->clear();
}

template <>
Map<SensorMap, SensorValue>::Map(const string &nameSpace, const int opts)
    : ManagedShmem(nameSpace, opts) {
  mapImpl = memory->find<SensorMap>(string(nameSpace + "map").c_str()).first;
  if (mapImpl == nullptr) {
    throw BadMapException();
  }
}

template <> vector<SensorValue> Map<SensorMap, SensorValue>::getAllValues() {
  vector<SensorValue> values;
  SensorValue value;
  TryReadLock();
  auto itr = mapImpl->begin();
  for (; itr != mapImpl->end(); itr++) {
    value = (*itr).second;
    values.emplace_back(std::move(value));
  }
  return values;
}

template <> Map<SensorMap, SensorValue>::~Map() {
  if (opts & O_CREAT) {
    memory->destroy<SensorMap>(string(nameSpace + "map").c_str());
  }
}

template <>
bool Map<SensorMap, SensorValue>::getValue(const string &key,
                                           SensorValue &val) {
  TryReadLock();
  auto itr = mapImpl->find(getMapKey(key));
  if (itr != mapImpl->end()) {
    val = (*itr).second;
    return true;
  } else {
    return false;
  }
}

template <>
bool Map<SensorMap, SensorValue>::updateTimestamp(const string &key,
                                                  const uint64_t timestamp,
                                                  const string &timestampStr) {
  if (opts & O_CREAT) {
    shmem_write_lock_t lock(*memLock);
    auto itr = mapImpl->find(getMapKey(key));
    if (itr != mapImpl->end()) {
      (*itr).second.timestamp = timestamp;
      (*itr).second.timestampStr = timestampStr;
      return true;
    } else {
      return false;
    }
  } else {
    throw PermissionErrorException();
  }
}

template <>
bool Map<SensorMap, SensorValue>::updateValue(const string &key,
                                              const string &val) {
  if (opts & O_CREAT) {
    shmem_write_lock_t lock(*memLock);
    auto itr = mapImpl->find(getMapKey(key));
    if (itr != mapImpl->end()) {
      (*itr).second.sensorValue = val;
      return true;
    } else {
      return false;
    }
  } else {
    throw PermissionErrorException();
  }
}

template <>
void Map<SensorMap, SensorValue>::insert(const string &key,
                                         const SensorValue &val) {
  if (opts & O_CREAT) {
    shmem_write_lock_t lock(*memLock);
    SensorMapValue mapValue(*voidAllocator);
    mapValue.metricProperty = val.metricProperty;
    mapValue.timestampStr = val.timestampStr;
    mapValue.sensorValue = val.sensorValue;
    mapValue.timestamp = val.timestamp;
    map_value_type_t mapEntry(getMapKey(key), mapValue);
    mapImpl->insert(mapEntry);
  } else {
    throw PermissionErrorException();
  }
}

template <>
bool Map<SensorMap, SensorValue>::updateValueAndTimeStamp(
    const string &key, const string &val, const uint64_t timestamp,
    const string &timestampStr) {

  if (opts & O_CREAT) {
    shmem_write_lock_t lock(*memLock);
    auto itr = mapImpl->find(getMapKey(key));
    if (itr != mapImpl->end()) {
      (*itr).second.sensorValue = val;
      (*itr).second.timestamp = timestamp;
      (*itr).second.timestampStr = timestampStr;
      return true;
    } else {
      return false;
    }
  } else {
    throw PermissionErrorException();
  }
}