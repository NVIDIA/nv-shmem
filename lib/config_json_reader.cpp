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

#include "impl/config_json_reader.hpp"

#include <stdexcept>

using namespace nv::shmem;

unique_ptr<Json> ConfigReader::namespaceCfgJson = nullptr;
unique_ptr<Json> ConfigReader::shmMappingJson = nullptr;

void ConfigReader::loadNamespaceConfig()
{
    if (namespaceCfgJson != nullptr)
    {
        return;
    }
    if (!filesystem::exists(SHM_NAMESPACE_CFG_JSON))
    {
        string errorMessage = "SHMEMDEBUG: namespaceCfg Json file" +
                              string(SHM_NAMESPACE_CFG_JSON) + "not present";
        LOG_ERROR(errorMessage);
        throw invalid_argument("Invalid filepath");
    }
    std::ifstream jsonFile(SHM_NAMESPACE_CFG_JSON);
    auto data = Json::parse(jsonFile, nullptr, false);
    if (data.is_discarded())
    {
        string errorMessage =
            "SHMEMDEBUG: Parsing namespaceCfg Json file failed, FILE=" +
            string(SHM_NAMESPACE_CFG_JSON);
        LOG_ERROR(errorMessage);
        throw runtime_error("Parsing namespaceCfg Json file failed");
    }
    SHMDEBUG("SHMEMDEBUG: NamespaceConfig loaded successfully: {JSONPATH}",
             "JSONPATH", string(SHM_NAMESPACE_CFG_JSON));
    namespaceCfgJson = make_unique<Json>(std::move(data));
}

void ConfigReader::loadSHMMappingConfig()
{
    if (shmMappingJson != nullptr)
    {
        return;
    }
    if (!filesystem::exists(SHM_MAPPING_JSON))
    {
        string errorMessage = "SHMEMDEBUG: shmMapping Json file" +
                              string(SHM_MAPPING_JSON) + "not present";
        LOG_ERROR(errorMessage);
        throw invalid_argument("Invalid filepath");
    }
    std::ifstream jsonFile(SHM_MAPPING_JSON);
    auto data = Json::parse(jsonFile, nullptr, false);
    if (data.is_discarded())
    {
        string errorMessage =
            "SHMEMDEBUG: Parsing shmMapping Json file failed, FILE=" +
            string(SHM_MAPPING_JSON);
        LOG_ERROR(errorMessage);
        throw runtime_error("Parsing shmMapping Json file failed");
    }
    SHMDEBUG("SHMEMDEBUG: SHMMapping loaded successfully: {JSONPATH}",
             "JSONPATH", string(SHM_MAPPING_JSON));
    shmMappingJson = make_unique<Json>(std::move(data));
}

unordered_map<string, vector<string>> ConfigReader::getProducers()
{
    if (shmMappingJson == nullptr)
    {
        string errorMessage = "SHMEMDEBUG: Json file is not loaded";
        LOG_ERROR(errorMessage);
        throw runtime_error("Json file is not loaded");
    }
    static unordered_map<string, vector<string>> producers;
    if (shmMappingJson->contains("Namespaces"))
    {
        for (const auto& namespaceEntry :
             (*shmMappingJson)["Namespaces"].items())
        {
            if (namespaceEntry.value().contains("Producers"))
                producers.emplace(make_pair(
                    namespaceEntry.key(), namespaceEntry.value()["Producers"]));
        }
    }
    else
    {
        string errorMessage =
            "SHMEMDEBUG: SHM Mapping file does not contain key Namespaces";
        LOG_ERROR(errorMessage);
        throw runtime_error("Namespaces key not found");
    }
    return producers;
}

NameSpaceConfiguration ConfigReader::getNameSpaceConfiguration()
{
    if (namespaceCfgJson == nullptr)
    {
        string errorMessage = "SHMEMDEBUG: Json file is not loaded";
        LOG_ERROR(errorMessage);
        throw runtime_error("Json file is not loaded");
    }
    NameSpaceConfiguration nameSpaceConfig;
    if (namespaceCfgJson->contains("SensorNamespaces"))
    {
        for (const auto& sensorNamespaceEntry :
             (*namespaceCfgJson)["SensorNamespaces"])
        {
            if (sensorNamespaceEntry.contains("Namespace") &&
                sensorNamespaceEntry.contains("ObjectpathKeywords") &&
                sensorNamespaceEntry.contains("PropertyList"))
            {
                const auto& sensorNamespace = sensorNamespaceEntry["Namespace"];
                const auto& objectpathKeywords =
                    sensorNamespaceEntry["ObjectpathKeywords"];
                const auto& propertyList = sensorNamespaceEntry["PropertyList"];
                NameSpaceValue namespaceVal = make_pair(objectpathKeywords,
                                                        propertyList);
                if (nameSpaceConfig.find(sensorNamespace) ==
                    nameSpaceConfig.end())
                {
                    NameSpaceValues namespaceValues = {namespaceVal};
                    nameSpaceConfig[sensorNamespace] =
                        std::move(namespaceValues);
                }
                else
                {
                    nameSpaceConfig[sensorNamespace].emplace_back(
                        std::move(namespaceVal));
                }
            }
            else
            {
                string errorMessage =
                    "SHMEMDEBUG: Invalid entry for shared memory namespace";
                LOG_ERROR(errorMessage);
                // Error in one entry continue with remaining entries
            }
        }
    }
    return nameSpaceConfig;
}

size_t ConfigReader::getSHMSize(const std::string& sensorNamespace,
                                const std::string& producerName)
{
    if (shmMappingJson == nullptr)
    {
        string errorMessage = "SHMEMDEBUG: Json file is not loaded";
        LOG_ERROR(errorMessage);
        throw runtime_error("Json file is not loaded");
    }
    if (shmMappingJson->contains("Namespaces"))
    {
        if ((*shmMappingJson)["Namespaces"].contains(sensorNamespace))
        {
            const auto& namespaceEntry =
                (*shmMappingJson)["Namespaces"][sensorNamespace];
            if (namespaceEntry.contains("Producers"))
            {
                const auto& producers =
                    namespaceEntry["Producers"].get<std::vector<string>>();
                if (find(producers.begin(), producers.end(), producerName) !=
                    producers.end())
                {
                    return namespaceEntry["SizeInBytes"];
                }
                else
                {
                    string errorMessage =
                        "SHMEMDEBUG: Namespace" + sensorNamespace +
                        "does not contain Producer" + producerName;
                    LOG_ERROR(errorMessage);
                    throw runtime_error("Key not found");
                }
            }
            else
            {
                string errorMessage = "SHMEMDEBUG: Namespace" +
                                      sensorNamespace +
                                      "does not contain Producers key";
                LOG_ERROR(errorMessage);
                throw runtime_error("Producers key not found");
            }
        }
        else
        {
            string errorMessage = "SHMEMDEBUG: Namespace" + sensorNamespace +
                                  "not found in mapping file";
            LOG_ERROR(errorMessage);
            throw runtime_error("Namespace not found");
        }
    }
    else
    {
        string errorMessage =
            "SHMEMDEBUG: SHM Mapping file does not contain key Namespaces";
        LOG_ERROR(errorMessage);
        throw runtime_error("Namespaces key not found");
    }
}

unordered_map<string, vector<string>> ConfigReader::getMRDNamespaceLookup()
{
    unordered_map<string, vector<string>> mrdNamespaceLookup;
    try
    {
        ConfigReader::loadSHMMappingConfig();
        try
        {
            const auto& producers = ConfigReader::getProducers();
            for (const auto& producerEntry : producers)
            {
                auto sensorNamespace = PLATFORMDEVICEPREFIX +
                                       producerEntry.first + "_0";
                mrdNamespaceLookup.emplace(
                    make_pair(move(sensorNamespace), producerEntry.second));
            }
        }
        catch (const exception& e)
        {
            lg2::error(
                "SHMEMDEBUG: Exception {EXCEPTION} while getting producers config. ",
                "EXCEPTION", e.what());
        }
    }
    catch (const exception& e)
    {
        lg2::error(
            "SHMEMDEBUG: Exception {EXCEPTION} while loading SHM Config. ",
            "EXCEPTION", e.what());
    }
    return mrdNamespaceLookup;
}
