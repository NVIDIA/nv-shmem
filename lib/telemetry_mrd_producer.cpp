#include "telemetry_mrd_producer.hpp"
#include "impl/shm_sensor_aggregator.hpp"

using namespace std;
using namespace nv::shmem;

shared_ptr<SHMSensorAggregator> AggregationService::sensorAggregator = nullptr;

bool AggregationService::namespaceInit(string processName) {
  AggregationService::sensorAggregator =
      make_unique<SHMSensorAggregator>(std::move(processName));
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
    return sensorAggregator->updateNanValue(devicePath, interface, propName,
                                            timestamp);
  } else {
    return sensorAggregator->updateSHMObject(devicePath, interface, propName,
                                             value, timestamp,
                                             associatedEntityPath);
  }
}