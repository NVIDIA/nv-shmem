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

#pragma once

#include <shm_common.h>

#include <nlohmann/json.hpp>
#include <phosphor-logging/lg2.hpp>

#include <fstream>
#include <unordered_map>

namespace nv {
namespace shmem {
using namespace std;
using Json = nlohmann::json;
using SensorNameSpace = string;
using ObjectpathKeywords = string;
using PropertyList = vector<string>;
using NameSpaceValue = pair<ObjectpathKeywords, PropertyList>;
using NameSpaceValues = vector<NameSpaceValue>;
using NameSpaceConfiguration = unordered_map<SensorNameSpace, NameSpaceValues>;

struct ConfigReader {
private:
  static std::unique_ptr<Json> namespaceCfgJson;
  static std::unique_ptr<Json> shmMappingJson;

public:
  /**
   * @brief This method loads sensor namespace configuration file which has
   * object path and property list for all sensor naemspaces. Namespace config
   * needs to be loaded only for producer and namespace json config is not
   * required for client.
   *
   * @throws std::exception If the file cannot be opened (for example, if the
   * file does not exist or the application does not have the necessary
   * permissions to open it), or if the file content is not valid JSON.
   */
  static void loadNamespaceConfig();

  /**
   * @brief This method loads shared memory mapping configuration file which has
   * all producer names and shared memory max size configuration.
   *
   * @throws std::exception If the file cannot be opened (for example, if the
   * file does not exist or the application does not have the necessary
   * permissions to open it), or if the file content is not valid JSON.
   */
  static void loadSHMMappingConfig();
  /**
   * @brief Get the Producers entries from config file
   *
   * @return unordered_map<string, vector<string>> - namespace and producer
   * @throws std::exception if json file is not loaded
   * names
   */
  static unordered_map<string, vector<string>> getProducers();

  /**
   * @brief This method parses property mapping entries from
   * namespace_lookup.config  file and returns parsed NameSpaceConfiguration
   *
   * @return NameSpaceConfiguration - namespace config entries
   */
  static NameSpaceConfiguration getNameSpaceConfiguration();

  /**
   * @brief Method to get shared memory size for a sensor namespace from shared
   * memory mapping file.
   *
   * @param[in] sensorNamespace - sensor namespace
   * @param[in] producerName - producer name
   * @return size_t
   * @throws std::exception if there are parsing errors or key is not found
   */
  static size_t getSHMSize(const std::string &sensorNamespace,
                           const std::string &producerName);

  /**
   * @brief Method to get MRDNamspaceLookup config from shared memory mapping
   * file. This is a static method and called only once during first look up.
   *
   * @return unordered_map<string, vector<string>>
   * @throws std::exception if there are parsing errors or json errors
   */
  static unordered_map<string, vector<string>> getMRDNamespaceLookup();
};
} // namespace shmem
} // namespace nv