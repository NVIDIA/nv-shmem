#include "telemetry_mrd_client.hpp"
#include "config.h"
#include "impl/shm_sensormap_intf.hpp"
#include "impl/shmem_map.hpp"
#include <phosphor-logging/lg2.hpp>
#include <string>
#include <unordered_map>

using namespace std;

namespace nv {
namespace shmem {
namespace sensor_aggregation {
// We need to prepare this from JSON Config file
unordered_map<string, vector<string>> mrdNamespaceLookup = {
    {PLATFORMDEVICEPREFIX + string("PlatformEnvironmentMetrics_0"),
     {"gpumgrd", "pldmd", "hwmontemp"}},
    {PLATFORMDEVICEPREFIX + string("NVSwitchMetrics_0"), {"gpumgrd"}},
    {PLATFORMDEVICEPREFIX + string("NVSwitchPortMetrics_0"), {"gpumgrd"}},
    {PLATFORMDEVICEPREFIX + string("MemoryMetrics_0"), {"gpumgrd"}},
    {PLATFORMDEVICEPREFIX + string("ProcessorMetrics_0"), {"gpumgrd"}},
    {PLATFORMDEVICEPREFIX + string("ProcessorGPMMetrics_0"), {"gpumgrd"}},
    {PLATFORMDEVICEPREFIX + string("ProcessorPortMetrics_0"), {"gpumgrd"}},
    {PLATFORMDEVICEPREFIX + string("ProcessorPortGPMMetrics_0"), {"gpumgrd"}}};

vector<SensorValue> getAllMRDValues(const string &mrdNamespace) {
  static unordered_map<string, unique_ptr<sensor_map_type>> sensor_map;
  vector<SensorValue> values;
  if (mrdNamespaceLookup.find(mrdNamespace) != mrdNamespaceLookup.end()) {
    for (auto &producerName : mrdNamespaceLookup[mrdNamespace]) {
      try {
        auto nameSpace = producerName + "_" + mrdNamespace;
        if (sensor_map.find(nameSpace) == sensor_map.end()) {
          sensor_map.insert(std::make_pair(
              nameSpace, make_unique<sensor_map_type>(nameSpace, O_RDONLY)));
        }
        const auto &mrdValues = sensor_map[nameSpace]->getAllValues();
        if (mrdValues.size()) {
          lg2::info("Requested {MRD} namespace has {NUMBER} of elements", "MRD",
                    nameSpace, "NUMBER", mrdValues.size());
          values.insert(values.end(), mrdValues.begin(), mrdValues.end());
        } else {
          lg2::error("Requested {MRD} namespace has no elements", "MRD",
                     nameSpace);
        }
      } catch (exception const &e) {
        lg2::error("Exception {EXCEPTION} while reading from {MRD} namespace",
                   "EXCEPTION", e.what(), "MRD", mrdNamespace);
      }
    }
    if (values.size() != 0) {
      return values;
    } else {
      lg2::error("Requested {MRD} namespace has no elements.", "MRD",
                 mrdNamespace);
      throw NoElementsException();
    }
  } else {
    lg2::error("Requested {MRD} namespace is not found in the MRD lookup.",
               "MRD", mrdNamespace);
    throw NameSpaceNotFoundException();
  }
}
} // namespace sensor_aggregation
} // namespace shmem
} // namespace nv