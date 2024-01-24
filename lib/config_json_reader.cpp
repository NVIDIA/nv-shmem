#include "impl/config_json_reader.hpp"
#include "config.h"
#include <stdexcept>

using namespace nv::shmem;

unique_ptr<Json> ConfigReader::namespaceCfgJson = nullptr;
unique_ptr<Json> ConfigReader::shmMappingJson = nullptr;

void ConfigReader::loadNamespaceConfig() {
  if (namespaceCfgJson != nullptr) {
    return;
  }
  if (!filesystem::exists(SHM_NAMESPACE_CFG_JSON)) {
    lg2::error("SHMEMDEBUG: namespaceCfg Json file {JSONPATH} not present",
               "JSONPATH", string(SHM_NAMESPACE_CFG_JSON));
    throw invalid_argument("Invalid filepath");
  }
  std::ifstream jsonFile(SHM_NAMESPACE_CFG_JSON);
  auto data = Json::parse(jsonFile, nullptr, false);
  if (data.is_discarded()) {
    lg2::error(
        "SHMEMDEBUG: Parsing namespaceCfg Json file failed, FILE={JSONPATH}",
        "JSONPATH", string(SHM_NAMESPACE_CFG_JSON));
    throw runtime_error("Parsing namespaceCfg Json file failed");
  }
  lg2::info("SHMEMDEBUG: NamespaceConfig loaded successfully: {JSONPATH}",
            "JSONPATH", string(SHM_NAMESPACE_CFG_JSON));
  namespaceCfgJson = make_unique<Json>(std::move(data));
}

void ConfigReader::loadSHMMappingConfig() {
  if (shmMappingJson != nullptr) {
    return;
  }
  if (!filesystem::exists(SHM_MAPPING_JSON)) {
    lg2::error("SHMEMDEBUG: shmMapping Json file {JSONPATH} not present",
               "JSONPATH", string(SHM_MAPPING_JSON));
    throw invalid_argument("Invalid filepath");
  }
  std::ifstream jsonFile(SHM_MAPPING_JSON);
  auto data = Json::parse(jsonFile, nullptr, false);
  if (data.is_discarded()) {
    lg2::error(
        "SHMEMDEBUG: Parsing shmMapping Json file failed, FILE={JSONPATH}",
        "JSONPATH", string(SHM_MAPPING_JSON));
    throw runtime_error("Parsing shmMapping Json file failed");
  }
  lg2::info("SHMEMDEBUG: SHMMapping loaded successfully: {JSONPATH}",
            "JSONPATH", string(SHM_MAPPING_JSON));
  shmMappingJson = make_unique<Json>(std::move(data));
}

unordered_map<string, vector<string>> ConfigReader::getProducers() {
  if (shmMappingJson == nullptr) {
    lg2::error("SHMEMDEBUG: Json file is not loaded");
    throw runtime_error("Json file is not loaded");
  }
  static unordered_map<string, vector<string>> producers;
  if (shmMappingJson->contains("Namespaces")) {
    for (const auto &namespaceEntry : (*shmMappingJson)["Namespaces"].items()) {
      if (namespaceEntry.value().contains("Producers"))
        producers.emplace(make_pair(namespaceEntry.key(),
                                    namespaceEntry.value()["Producers"]));
    }
  } else {
    lg2::error("SHMEMDEBUG: SHM Mapping file does not contain key Namespaces");
    throw runtime_error("Namespaces key not found");
  }
  return producers;
}

NameSpaceConfiguration ConfigReader::getNameSpaceConfiguration() {
  if (namespaceCfgJson == nullptr) {
    lg2::error("SHMEMDEBUG: Json file is not loaded");
    throw runtime_error("Json file is not loaded");
  }
  NameSpaceConfiguration nameSpaceConfig;
  if (namespaceCfgJson->contains("SensorNamespaces")) {
    for (const auto &sensorNamespaceEntry :
         (*namespaceCfgJson)["SensorNamespaces"]) {
      if (sensorNamespaceEntry.contains("Namespace") &&
          sensorNamespaceEntry.contains("ObjectpathKeywords") &&
          sensorNamespaceEntry.contains("PropertyList")) {
        const auto &sensorNamespace = sensorNamespaceEntry["Namespace"];
        const auto &objectpathKeywords =
            sensorNamespaceEntry["ObjectpathKeywords"];
        const auto &propertyList = sensorNamespaceEntry["PropertyList"];
        NameSpaceValue namespaceVal =
            make_pair(objectpathKeywords, propertyList);
        if (nameSpaceConfig.find(sensorNamespace) == nameSpaceConfig.end()) {
          NameSpaceValues namespaceValues = {namespaceVal};
          nameSpaceConfig[sensorNamespace] = std::move(namespaceValues);
        } else {
          nameSpaceConfig[sensorNamespace].emplace_back(
              std::move(namespaceVal));
        }
      } else {
        lg2::error("SHMEMDEBUG: Invalid entry for shared memory namespace");
        // Error in one entry continue with remaining entries
      }
    }
  }
  return nameSpaceConfig;
}

size_t ConfigReader::getSHMSize(const std::string &sensorNamespace,
                                const std::string &producerName) {
  if (shmMappingJson == nullptr) {
    lg2::error("SHMEMDEBUG: Json file is not loaded");
    throw runtime_error("Json file is not loaded");
  }
  if (shmMappingJson->contains("Namespaces")) {
    if ((*shmMappingJson)["Namespaces"].contains(sensorNamespace)) {
      const auto &namespaceEntry =
          (*shmMappingJson)["Namespaces"][sensorNamespace];
      if (namespaceEntry.contains("Producers")) {
        const auto &producers =
            namespaceEntry["Producers"].get<std::vector<string>>();
        if (find(producers.begin(), producers.end(), producerName) !=
            producers.end()) {
          return namespaceEntry["SizeInBytes"];
        } else {
          lg2::error(
              "SHMEMDEBUG: Namespace {SENSOR_NAMESPACE} does not contain "
              "Producer {PRODUCER_NAME}",
              "SENSOR_NAMESPACE", sensorNamespace, "PRODUCER_NAME",
              producerName);
          throw runtime_error("Key not found");
        }
      } else {
        lg2::error("SHMEMDEBUG: Namespace {SENSOR_NAMESPACE} does not contain "
                   "Producers key",
                   "SENSOR_NAMESPACE", sensorNamespace);
        throw runtime_error("Producers key not found");
      }
    } else {
      lg2::error(
          "SHMEMDEBUG: Namespace {SENSOR_NAMESPACE} not found in mapping file",
          "SENSOR_NAMESPACE", sensorNamespace);
      throw runtime_error("Namespace not found");
    }
  } else {
    lg2::error("SHMEMDEBUG: SHM Mapping file does not contain key Namespaces");
    throw runtime_error("Namespaces key not found");
  }
}

unordered_map<string, vector<string>> ConfigReader::getMRDNamespaceLookup() {
  unordered_map<string, vector<string>> mrdNamespaceLookup;
  try {
    ConfigReader::loadSHMMappingConfig();
    try {
      const auto &producers = ConfigReader::getProducers();
      for (const auto &producerEntry : producers) {
        auto sensorNamespace =
            PLATFORMDEVICEPREFIX + producerEntry.first + "_0";
        mrdNamespaceLookup.emplace(
            make_pair(move(sensorNamespace), producerEntry.second));
      }
    } catch (exception const &e) {
      lg2::error(
          "SHMEMDEBUG: Exception {EXCEPTION} while getting producers config. ",
          "EXCEPTION", e.what());
    }

  } catch (exception const &e) {
    lg2::error("SHMEMDEBUG: Exception {EXCEPTION} while loading SHM Config. ",
               "EXCEPTION", e.what());
  }
  return mrdNamespaceLookup;
}