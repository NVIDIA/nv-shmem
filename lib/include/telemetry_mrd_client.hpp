/*
Methods defined in this header provides APIs to get all objects from a MRD
namespace. Based on the MRD requested in the URI, the library creates a list of
shared memory namespaces whose metric type matches with the requested metric
type in the URI. From each of the shared memory regions read all objects and
aggregate them.

Below are some examples on API usage, refer API documentation section in this
file for more details.

getAllMRDValues
*******************************************************************************
This API returns all metric report definitions for a given namespace. This API
should be used by MRD clients such as bmcweb. Exceptions will be thrown in case
of no elements or absense of given shared memory namespace.

Example:
-------------------------------------------------------------------------------
    std::string metricId = "PlatformEnvironmentMetrics";
    const auto& values =
nv::shmem::sensor_aggregation::getAllMRDValues(metricId);
*/
#pragma once
#include <shm_common.h>
#include <vector>

namespace nv {
namespace shmem {
namespace sensor_aggregation {

/**
 * @brief This API returns all metric report definitions for a given namespace.
 * This API should be used by MRD clients such as bmcweb. Exceptions will be
 * thrown in case of no elements or absense of given shared memory namespace.
 *
 * @param[in] mrdNamespace - metric report definitions namespace
 * @return values - metric report definitions values. Incase of no elements or
 * absence of given shared memory namespace exception is thrown.
 */
std::vector<SensorValue> getAllMRDValues(const std::string &mrdNamespace);

/**
 * @brief This exception should be thrown when name space is not found in shared
 * memory.
 *
 */
struct NameSpaceNotFoundException : public std::runtime_error {
  NameSpaceNotFoundException()
      : std::runtime_error("Namespace is not found in shared memory") {}
};

/**
 * @brief This exception should be thrown when there are no elements in shared
 * memory for a given namespace
 *
 */
struct NoElementsException : public std::runtime_error {
  NoElementsException()
      : std::runtime_error("Namespace has no elements in shared memory") {}
};

} // namespace sensor_aggregation
} // namespace shmem
} // namespace nv