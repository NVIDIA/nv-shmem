# Library for Shared Memory based Sensor Aggregation

Author: Chinmay Shripad Hegde <chinmays@nvidia.com> Rohit Pai <ropai@nvidia.com>

Created: 2023-12-27

## Problem Description

In the existing architecture bmcweb makes Object manager based D-Bus calls and
processes all the properties in the response to filter out and prepare the
properties required for MRD URIs. With this approach we have performance impacts
such as % PID loop outliers and maximum MRD TAT crossing the SLAs.

## Background and References

Shared memory sensor repository acts like a cache for bmweb where all the sensor
data and metric values required to prepare the response for MRD requests are
readily available. The repository is periodically updated by the sensor
aggregator library which gets the data from individual sensor producers. Sensor
producers will use shared memory APIs to insert and update the MRD values.This
brings in significant reduction in processing time of MRD requests in bmcweb
which will help to improve the SLAs.

### References

| Content                                             | Link                                                                                                               |
| --------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------ |
| Redfish Telemetry Performance                       | https://docs.google.com/presentation/d/1Srm3HF2QK7uQGdDESiIBHUAMPdG0nOXfHvY94QH18uo/edit#slide=id.g25866484049_0_0 |
| TAL Fasibility Study and Recommendation for Umbriel | https://docs.google.com/document/d/1UanEEKkR5ffHC1_QCb55f9HONJ3zH9e8ocVhRK0gc6Q/edit#heading=h.vvyjokuu77rq        |
| Requirements                                        | https://docs.google.com/document/d/1W--n4Mu_twg8rsMzq8JtJmyqOaWnEVHluvHbr0BVvEE/edit                               |
| Telemetry SLAs                                      | https://docs.google.com/document/d/1nP8F1Xc6kB_qU5xSr0NDQ5lQL1bTkEux0O3Rrmg8PUw/edit#heading=h.1jwg24jxwkrr        |

## Requirements

### External API requirements

- API for data producer to update telemetry objects in shared memory.
- API for client such as bmcweb to read telemetry objects for a given metric
  namespace.

### Shared Memory internal requirements:

- Shmem library must support creation of shared memory segments which can be
  used as fast IPC between multiple processes.
- The Shmem library must abstract the internal memory layout required to save
  the objects.
- Shmem library must support unified object access APIs.
- Shemem library must support efficient access to single object.
- Shemem library APIs must ensure data integrity and protection in a multi
  process environment.
- Shemem library must provide CLI tool which can be invoked from shell to read,
  updated and erase objects in the shared memory segments.

## Proposed Design

```ascii
                    ┌─────────────────────────────────┐
                    │                                 │
┌────────┐          │  ┌──────────────────────────┐   │
│        │          │  │                          │   │
│ Client ├──────────┼──►    getAllMRDValues       │   │
│        │          │  │                          │   │
└────────┘          │  └────────────▲─────────────┘   │
                    │               │                 │             ┌─────────────────┐
                    │               └──────Read───────┼─────────────►                 │
                    │               ┌──────Write──────┼─────────────►   Shared Memory │
                    │               │                 │             └─────────────────┘
                    │  ┌────────────┴─────────────┐   │
┌────────────┐      │  │    - namespaceInit       │   │
│            │      │  │    - updateTelemetry     │   │
│Producer - 1├───┬──┼──►                          │   │
│            │   │  │  └──────────────────────────┘   │
└────────────┘   │  │                                 │
                 │  │  Shared Memory Library          │
┌────────────┐   │  │                                 │
│            │   │  └─────────────────────────────────┘
│Producer - 2├───┘
│            │
└────────────┘
```

### Sensor Producer Workflow

Producers will use the sensor aggregator library APIs to

- Initialize the producer name
- Create Sensor Objects in the shared memory
- Update Value and/OR Timestamp values

### Bmcweb Workflow

- Sensor Aggregator library provides an API to get all objects from a MRD
  namespace.
- Based on the MRD requested in the URI, the library creates a list of shared
  memory namespaces whose metric type matches with the requested metric type in
  the URI. From each of the shared memory regions read all objects and aggregate
  them.

### Configuration json for metric property mapping

There would be a configuration file which maps MRD namespace to all the shared
memory namespaces exposed by each of the sensor provider services.
`SensorNamespaces` key contains rules for each of the sensor namespace which is
combination of `Namespace`, `ObjectpathKeywords` and `PropertyList`.

Note: By default `shm_namespace_config.json` file present in configurations
directory will be used. Override this file in your platform recipe file based on
the requirement.

An example of this configuration file is given below.

```ascii
{
  "SensorNamespaces": [
    {
        "Namespace": "PlatformEnvironmentMetrics",
        "ObjectpathKeywords": "sensors/temperature",
        "PropertyList": ["Value"]
    },
    {
        "Namespace": "MemoryMetrics",
        "ObjectpathKeywords": "memory",
        "PropertyList": [
            "MemoryConfiguredSpeedInMhz",
            "Utilization",
            "ceCount",
            "ueCount",
            "RowRemappingFailureState",
            "ceRowRemappingCount",
            "ueRowRemappingCount"
        ]
    }
  ]
}

```

### Shared memory mapping json

This file will have updating producer names for all the sensor namespaces. For
each namespace

- `Producers` key will contain list of processes which updates shared memory for
  the given namespace.
- Shared memory size should be specified in bytes with field `SizeInBytes`

Note: By default `shm_mapping.json` file present in configurations directory
will be used. Override this file in your platform recipe file based on the
requirement.

SizeInBytes is currently defined as below. The calculation is based on used size
in HGX platform for each of the metrics namespaces.

Used size is calculated using `managed_shared_memory.get_free_memory()` api.
Based on the used size it's adjusted to it's near 2's power value in bytes.

| Namespace                        | Size in system KB | Allocated size |
| -------------------------------- | ----------------- | -------------- |
| HGX_MemoryMetrics_0              | 46.1328125        | 128 KB         |
| HGX_NVSwitchMetrics_0            | 40.4453125        | 128 KB         |
| HGX_NVSwitchPortMetrics_0        | 894.4765625       | 1024 KB        |
| HGX_PlatformEnvironmentMetrics_0 | 55.921875         | 128 KB         |
| HGX_ProcessorGPMMetrics_0        | 138.0234375       | 256 KB         |
| HGX_ProcessorMetrics_0           | 103.2734375       | 256 KB         |
| HGX_ProcessorPortGPMMetrics_0    | 267.203125        | 512 KB         |
| HGX_ProcessorPortMetrics_0       | 829.7265625       | 1024 KB        |

An example of this namespace mapping is given below.

```
{
    "Namespaces": {
        "PlatformEnvironmentMetrics": {
            "Producers": ["gpumgrd", "pldmd"],
            "SizeInBytes": 1024000
        }
    }
}
```

## Usage

### Shared memory client APIs

This API returns all metric report definitions for a given namespace. This API
should be used by MRD clients such as bmcweb. Exceptions will be thrown in case
of no elements or absense of given shared memory namespace.

```ascii
API:
std::vector<SensorValue> getAllMRDValues(const string &mrdNamespace);

Example:
std::string metricId = "PlatformEnvironmentMetrics";
const auto& values = nv::shmem::sensor_aggregation::getAllMRDValues(metricId);
```

### Shared memory producer APIs

#### Init namespace

This API is to initialize the namespace. This API takes the process name as
input which will be the suffix for shared memory namespaces. This API should be
called once in telemetry producers, before creating new telemetry objects.

API:

```ascii

static bool AggregationService::namespaceInit(const std::string& processName);
```

Parameters:

- processName - process name of producer service

Example:

```ascii
std::string producerName = "gpumgrd";
AggregationService::namespaceInit(producerName);
```

#### Update telemetry

API to add new telemetry object, update existing telemetry object value and
update nan. This API should be called by telemetry producers.

All PlatformEnvironmentMetrics will be under /xyz/openbmc_project/sensors. This
API expects associationPath for sensors paths and returns error if
associationPath is not passed for the sensor objects.

API:

```ascii
static bool AggregationService::updateTelemetry(const std::string& devicePath,
                 const std::string& interface,
                 const std::string& propName,
                 DbusVariantType& value, const uint64_t timestamp, int rc,
                 const std::string associatedEntityPath = {});
```

Parameters:

- devicePath - Device path of telemetry object.
- interface - Phosphor D-Bus interface of telemetry object.
- propName - Metric name.
- value - Metric value.
- timestamp - Timestamp of telemetry object.
- rc - Set this value to non zero for nan update.
- associatedEntityPath - optional for other metrics. Required for platform
  environment metrics.

Example: Insert

```ascii
#include <telemetry_mrd_service.hpp>
using namespace nv::sensor_aggregation;

std::string devicePath =
    "/xyz/openbmc_project/sensors/temperature/HGX_Chassis_0_HSC_0_Temp_0";
std::string interface = "xyz.openbmc_project.Sensor.Value";
std::string propertyName = "Value";
std::string parentPath = "HGX_Chassis_0";
DbusVariantType value = 19.0625;
uint64_t timeStamp = 23140448;

if (AggregationService::updateTelemetry(devicePath, interface,
                                        propertyName, value, 0, 0,
                                        parentPath)) {
  std::cout << "AggregationService::updateTelemetry success for new object"
            << std::endl;
} else {
  std::cout << "AggregationService::updateTelemetry failed for new object"
            << std::endl;
}
```

Example: Update timestamp and value

```ascii
value = 29.0625;
timeStamp = 23150448
if (AggregationService::updateTelemetry(devicePath, interface,
                                        propertyName, value, 0, 0,
                                        parentPath)) {
  std::cout << "AggregationService::updateTelemetry value and timestamp "
                "update success"
            << std::endl;
} else {
  std::cout << "AggregationService::updateTelemetry value and timestamp "
                "update failed"
            << std::endl;
}

```

Example: Update nan

```ascii
if (AggregationService::updateTelemetry(
        devicePath, interface, propertyName, value, timeStamp, -1)) {
  std::cout << "AggregationService: nan handling simple type success"
            << std::endl;
} else {
  std::cout << "AggregationService: nan handling simple type failed"
            << std::endl;
}

```

## Telemetry Readiness

Individual producers update the status in CSM over D-Bus once all objects are
updated, CSM populates the overall telemetry readiness based on individual
producer state. Redfish clients should rely on Telemetry readiness before
querying the MRD properties.

Internal clients needs to explicitly check telemetry readiness from the CSM
module over dbus and when its ready they can fetch the MRD objects from the
shmem.

## Troubleshooting and logging

By default only informational and error logs are enabled. Debug logs can be
enabled with a meson flag `enable-shm-debug`. To add additional debug traces you
can use `SHMDEBUG` method.
