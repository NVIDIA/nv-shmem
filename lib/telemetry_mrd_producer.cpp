#include "telemetry_mrd_producer.hpp"
#include "impl/config_json_reader.hpp"
#include "impl/shm_sensor_aggregator.hpp"

using namespace std;
using namespace nv::shmem;

shared_ptr<SHMSensorAggregator> AggregationService::sensorAggregator = nullptr;

bool AggregationService::namespaceInit(string processName) {
  lg2::info("Initializing shm namespace for process: {PROCESS_NAME}",
            "PROCESS_NAME", processName);
  try {
    ConfigReader::loadSHMMappingConfig();
  } catch (exception const &e) {
    lg2::error(
        "SHMEMDEBUG: Exception {EXCEPTION} while loading shm mapping config. ",
        "EXCEPTION", e.what());
    return false;
  }
  try {
    ConfigReader::loadNamespaceConfig();
    try {
      const auto &nameSpaceCfg = ConfigReader::getNameSpaceConfiguration();
      AggregationService::sensorAggregator = make_unique<SHMSensorAggregator>(
          move(processName), move(nameSpaceCfg));
    } catch (exception const &e) {
      lg2::error(
          "SHMEMDEBUG: Exception {EXCEPTION} while reading namespace config. ",
          "EXCEPTION", e.what());
      return false;
    }

  } catch (exception const &e) {
    lg2::error(
        "SHMEMDEBUG: Exception {EXCEPTION} while loading namespace config. ",
        "EXCEPTION", e.what());
    return false;
  }
  return true;
}

bool AggregationService::updateTelemetry(const string &devicePath,
                                         const string &interface,
                                         const string &propName,
                                         DbusVariantType &value,
                                         const uint64_t timestamp, int rc,
                                         const string associatedEntityPath) {
  if (sensorAggregator == nullptr) {
    return false;
  }
  if (rc != 0 && timestamp != 0) {
    SHMDEBUG("SHMEMDEBUG: Updating NAN value for key "
             "{DEVICE_PATH}:{INTERFACE}:{PROPNAME}",
             "DEVICE_PATH", devicePath, "INTERFACE", interface, "PROPNAME",
             propName);
    return sensorAggregator->updateNanValue(devicePath, interface, propName,
                                            timestamp);
  } else {
    SHMDEBUG("SHMEMDEBUG: Updating Object for key "
             "{DEVICE_PATH}:{INTERFACE}:{PROPNAME}",
             "DEVICE_PATH", devicePath, "INTERFACE", interface, "PROPNAME",
             propName);
    return sensorAggregator->updateSHMObject(devicePath, interface, propName,
                                             value, timestamp,
                                             associatedEntityPath);
  }
}