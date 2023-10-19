#pragma once
#include <string>
#include <unordered_map>

using namespace std;

namespace nv {
namespace sensor_aggregation {
namespace metricUtils {

/* Map for link status pdi to redfish string */
static unordered_map<string, string> linkStatusTypeMap = {
    {"xyz.openbmc_project.Inventory.Item.Port.LinkStatusType.LinkDown",
     "LinkDown"},
    {"xyz.openbmc_project.Inventory.Item.Port.LinkStatusType.LinkUp", "LinkUp"},
    {"xyz.openbmc_project.Inventory.Item.Port.LinkStatusType.NoLink", "NoLink"},
    {"xyz.openbmc_project.Inventory.Item.Port.LinkStatusType.Starting",
     "Starting"},
    {"xyz.openbmc_project.Inventory.Item.Port.LinkStatusType.Training",
     "Training"}};

/**
 * @brief Method to Get the Link Status Type redfish string from PDI name.
 *
 * @param[in] linkStatusType
 * @return string
 */
inline string getLinkStatusType(const string &linkStatusType) {
  if (linkStatusTypeMap.find(linkStatusType) != linkStatusTypeMap.end()) {
    return linkStatusTypeMap[linkStatusType];
  }
  return "";
}

} // namespace metricUtils
} // namespace sensor_aggregation
} // namespace nv
