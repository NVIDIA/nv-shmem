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

#pragma once
#include "config.h"

#include "port_utils.hpp"
#include "time_utils.hpp"

#include <shm_common.h>

#include <boost/algorithm/string.hpp>

#include <cctype>
#include <string>
#include <unordered_map>

using namespace std;

namespace nv
{
namespace sensor_aggregation
{
namespace metricUtils
{

// Property data structures for Shared memory updates
using MetricProp = string;
using MetricVal = string;
using SHMKey = string;
using SHMValue = tuple<MetricVal, MetricProp>;
using MetricName = std::string;
using PDIName = std::string;
using MetricNameMap = unordered_map<MetricName, string>;
using PDINameMap = unordered_map<PDIName, MetricNameMap>;

/* Map for reason type PDI to redfish string */
static unordered_map<string, string> reasonTypeMap = {
    {"xyz.openbmc_project.State.ProcessorPerformance."
     "ThrottleReasons.SWPowerCap",
     "SWPowerCap"},
    {"xyz.openbmc_project.State.ProcessorPerformance."
     "ThrottleReasons.HWSlowdown",
     "HWSlowdown"},
    {"xyz.openbmc_project.State.ProcessorPerformance."
     "ThrottleReasons.HWThermalSlowdown",
     "HWThermalSlowdown"},
    {"xyz.openbmc_project.State.ProcessorPerformance."
     "ThrottleReasons.HWPowerBrakeSlowdown",
     "HWPowerBrakeSlowdown"},
    {"xyz.openbmc_project.State.ProcessorPerformance."
     "ThrottleReasons.SyncBoost",
     "SyncBoost"},
    {"xyz.openbmc_project.State.ProcessorPerformance."
     "ThrottleReasons.GPUThermalOvertTreshold",
     "Current GPU temperature above the GPU Max Operating Temperature or "
     "Current memory temperature above the Memory Max Operating "
     "Temperature"},
    {"xyz.openbmc_project.State.ProcessorPerformance.ThrottleReasons.None",
     "NA"},
    {"xyz.openbmc_project.State.ProcessorPerformance.PerformanceStates.Normal",
     "Normal"},
    {"xyz.openbmc_project.State.ProcessorPerformance.PerformanceStates.Throttled",
     "Throttled"},
    {"xyz.openbmc_project.State.ProcessorPerformance.PerformanceStates.Degraded",
     "Degraded"},
    {"xyz.openbmc_project.State.ProcessorPerformance.PerformanceStates.Unknown",
     "Unknown"}};

/* Map to convert pcie type pdi to redfish string */
static unordered_map<string, string> pcieTypeMap = {
    {"xyz.openbmc_project.Inventory.Item.PCIeDevice.PCIeTypes.Gen1", "Gen1"},
    {"xyz.openbmc_project.Inventory.Item.PCIeDevice.PCIeTypes.Gen2", "Gen2"},
    {"xyz.openbmc_project.Inventory.Item.PCIeDevice.PCIeTypes.Gen3", "Gen3"},
    {"xyz.openbmc_project.Inventory.Item.PCIeDevice.PCIeTypes.Gen4", "Gen4"},
    {"xyz.openbmc_project.Inventory.Item.PCIeDevice.PCIeTypes.Gen5", "Gen5"}};

/* Map to convert power state type pdi to redfish string */
static unordered_map<string, string> powerStateTypeMap = {
    {"xyz.openbmc_project.State.Decorator.OperationalStatus."
     "StateType.Absent",
     "Absent"},
    {"xyz.openbmc_project.State.Decorator.OperationalStatus."
     "StateType.Deferring",
     "Deferring"},
    {"xyz.openbmc_project.State.Decorator.OperationalStatus."
     "StateType.Disabled",
     "Disabled"},
    {"xyz.openbmc_project.State.Decorator.OperationalStatus."
     "StateType.Enabled",
     "Enabled"},
    {"xyz.openbmc_project.State.Decorator.OperationalStatus."
     "StateType.StandbyOffline",
     "StandbyOffline"},
    {"xyz.openbmc_project.State.Decorator.OperationalStatus."
     "StateType.Starting",
     "Starting"},
    {"xyz.openbmc_project.State.Decorator.OperationalStatus."
     "StateType.UnavailableOffline",
     "UnavailableOffline"},
    {"xyz.openbmc_project.State.Decorator.OperationalStatus."
     "StateType.Updating",
     "Updating"},
};

/* Map for portInfo interface pdi to redfish string based on metric name */
static MetricNameMap portInfoInterfaceMap = {
    {"CurrentSpeed", "#/CurrentSpeedGbps"}, {"MaxSpeed", "#/MaxSpeedGbps"}};

/* Map for portState interface pdi to redfish string based on metric name */
static MetricNameMap portStateInterfaceMap = {{"LinkStatus", "#/LinkStatus"},
                                              {"LinkState", "#/Status/State"}};

/* Map for IBPort interface pdi to redfish string based on metric name */
static MetricNameMap ibPortInterfaceMap = {
    {"RXErrors", "/Metrics#/RXErrors"},
    {"TXPkts", "/Metrics#/Networking/TXFrames"},
    {"RXPkts", "/Metrics#/Networking/RXFrames"},
    {"RXMulticastPkts", "/Metrics#/Networking/RXMulticastFrames"},
    {"TXMulticastPkts", "/Metrics#/Networking/TXMulticastFrames"},
    {"RXUnicastPkts", "/Metrics#/Networking/RXUnicastFrames"},
    {"TXUnicastPkts", "/Metrics#/Networking/TXUnicastFrames"},
    {"TXDiscardPkts", "/Metrics#/Networking/TXDiscards"},
    {"MalformedPkts", "/Metrics#/Oem/Nvidia/MalformedPackets"},
    {"VL15DroppedPkts", "/Metrics#/Oem/Nvidia/VL15Dropped"},
    {"VL15TXPkts", "/Metrics#/Oem/Nvidia/VL15TXPackets"},
    {"VL15TXData", "/Metrics#/Oem/Nvidia/VL15TXBytes"},
    {"MTUDiscard", "/Metrics#/Oem/Nvidia/NeighborMTUDiscards"},
    {"SymbolError", "/Metrics#/Oem/Nvidia/SymbolErrors"},
    {"LinkErrorRecoveryCounter", "/Metrics#/Oem/Nvidia/LinkErrorRecoveryCount"},
    {"LinkDownCount", "/Metrics#/Oem/Nvidia/LinkDownedCount"},
    {"RXRemotePhysicalErrorPkts",
     "/Metrics#/Oem/Nvidia/RXRemotePhysicalErrors"},
    {"RXSwitchRelayErrorPkts", "/Metrics#/Oem/Nvidia/RXSwitchRelayErrors"},
    {"QP1DroppedPkts", "/Metrics#/Oem/Nvidia/QP1Dropped"},
    {"BitErrorRate", "/Metrics#/Oem/Nvidia/BitErrorRate"},
    {"TXWait", "/Metrics#/Oem/Nvidia/TXWait"}};

/* Map for portMetricsOem1 interface pdi to redfish string based on metric name
 */
static MetricNameMap portMetricsOem1InterfaceMap = {
    {"DataCRCCount", "/Metrics#/Oem/Nvidia/NVLinkErrors/DataCRCCount"},
    {"FlitCRCCount", "/Metrics#/Oem/Nvidia/NVLinkErrors/FlitCRCCount"},
    {"RecoveryCount", "/Metrics#/Oem/Nvidia/NVLinkErrors/RecoveryCount"},
    {"ReplayErrorsCount", "/Metrics#/Oem/Nvidia/NVLinkErrors/ReplayCount"}};

/* Map for portMetricsOem2 interface pdi to redfish string based on metric name
 */
static MetricNameMap portMetricsOem2InterfaceMap = {
    {"RXBytes", "/Metrics#/RXBytes"}, {"TXBytes", "/Metrics#/TXBytes"}};

/* Map for portMetricsOem3 interface pdi to redfish string based on metric name
 */
static MetricNameMap portMetricsOem3InterfaceMap = {
    {"RXNoProtocolBytes", "/Metrics#/Oem/Nvidia/RXNoProtocolBytes"},
    {"TXNoProtocolBytes", "/Metrics#/Oem/Nvidia/TXNoProtocolBytes"},
    {"RuntimeError", "/Metrics#/Oem/Nvidia/NVLinkErrors/RuntimeError"},
    {"TrainingError", "/Metrics#/Oem/Nvidia/NVLinkErrors/TrainingError"},
    {"TXWidth", "#/Oem/Nvidia/TXWidth"},
    {"RXWidth", "#/Oem/Nvidia/RXWidth"}};

/* Map for processor performance pdi to redfish string based on metric name*/
static MetricNameMap processorPerfMap = {
    {"ThrottleReason", "/Oem/Nvidia/ThrottleReasons"},
    {"PowerLimitThrottleDuration", "/PowerLimitThrottleDuration"},
    {"ThermalLimitThrottleDuration", "/ThermalLimitThrottleDuration"},
    {"AccumulatedSMUtilizationDuration",
     "/Oem/Nvidia/AccumulatedSMUtilizationDuration"},
    {"AccumulatedGPUContextUtilizationDuration",
     "/Oem/Nvidia/AccumulatedGPUContextUtilizationDuration"},
    {"GlobalSoftwareViolationThrottleDuration",
     "/Oem/Nvidia/GlobalSoftwareViolationThrottleDuration"},
    {"HardwareViolationThrottleDuration",
     "/Oem/Nvidia/HardwareViolationThrottleDuration"},
    {"PCIeTXBytes", "/Oem/Nvidia/PCIeTXBytes"},
    {"PCIeRXBytes", "/Oem/Nvidia/PCIeRXBytes"},
    {"Value", "#/Oem/Nvidia/PowerBreakPerformanceState"}};

/* Map for NvLinkMetricsMap pdi to redfish string based on metric name*/
static MetricNameMap nvLinkMetricsMap = {
    {"NVLinkRawTxBandwidthGbps", "/Oem/Nvidia/NVLinkRawTxBandwidthGbps"},
    {"NVLinkRawRxBandwidthGbps", "/Oem/Nvidia/NVLinkRawRxBandwidthGbps"},
    {"NVLinkDataTxBandwidthGbps", "/Oem/Nvidia/NVLinkDataTxBandwidthGbps"},
    {"NVLinkDataRxBandwidthGbps", "/Oem/Nvidia/NVLinkDataRxBandwidthGbps"}};

/* Map for GPMMetrics pdi to redfish string based on metric name*/
static MetricNameMap gpmMetricsMap = {
    {"NVDecInstanceUtilizationPercent",
     "/Oem/Nvidia/NVDecInstanceUtilizationPercent"},
    {"NVJpgInstanceUtilizationPercent",
     "/Oem/Nvidia/NVJpgInstanceUtilizationPercent"},
    {"GraphicsEngineActivityPercent",
     "/Oem/Nvidia/GraphicsEngineActivityPercent"},
    {"SMActivityPercent", "/Oem/Nvidia/SMActivityPercent"},
    {"SMOccupancyPercent", "/Oem/Nvidia/SMOccupancyPercent"},
    {"TensorCoreActivityPercent", "/Oem/Nvidia/TensorCoreActivityPercent"},
    {"FP64ActivityPercent", "/Oem/Nvidia/FP64ActivityPercent"},
    {"FP32ActivityPercent", "/Oem/Nvidia/FP32ActivityPercent"},
    {"FP16ActivityPercent", "/Oem/Nvidia/FP16ActivityPercent"},
    {"NVDecUtilizationPercent", "/Oem/Nvidia/NVDecUtilizationPercent"},
    {"NVJpgUtilizationPercent", "/Oem/Nvidia/NVJpgUtilizationPercent"},
    {"NVOfaUtilizationPercent", "/Oem/Nvidia/NVOfaUtilizationPercent"},
    {"PCIeRawTxBandwidthGbps", "/Oem/Nvidia/PCIeRawTxBandwidthGbps"},
    {"PCIeRawRxBandwidthGbps", "/Oem/Nvidia/PCIeRawRxBandwidthGbps"},
    {"IntegerActivityUtilizationPercent",
     "/Oem/Nvidia/IntegerActivityUtilizationPercent"},
    {"DMMAUtilizationPercent", "/Oem/Nvidia/DMMAUtilizationPercent"},
    {"HMMAUtilizationPercent", "/Oem/Nvidia/HMMAUtilizationPercent"},
    {"IMMAUtilizationPercent", "/Oem/Nvidia/IMMAUtilizationPercent"},
};

/* Map for PCIeECC pdi to redfish string based on metric name*/
static MetricNameMap pcieECCMap = {
    {"nonfeCount", "/PCIeErrors/NonFatalErrorCount"},
    {"feCount", "/PCIeErrors/FatalErrorCount"},
    {"ceCount", "/PCIeErrors/CorrectableErrorCount"},
    {"PCIeECC.ceCount", "/PCIeErrors/CorrectableErrorCount"},
    {"L0ToRecoveryCount", "/PCIeErrors/L0ToRecoveryCount"},
    {"NAKReceivedCount", "/PCIeErrors/NAKReceivedCount"},
    {"ReplayCount", "/PCIeErrors/ReplayCount"},
    {"NAKSentCount", "/PCIeErrors/NAKSentCount"},
    {"ReplayRolloverCount", "/PCIeErrors/ReplayRolloverCount"},
    {"UnsupportedRequestCount", "/PCIeErrors/UnsupportedRequestCount"},
    {"PCIeType", "#/PCIeInterface/PCIeType"},
    {"MaxLanes", "#/PCIeInterface/MaxLanes"},
    {"LanesInUse", "#/PCIeInterface/LanesInUse"}};

/* Map for MemoryECC pdi to redfish string based on metric name*/
static MetricNameMap memoryECCMap = {{"ueCount", "/UncorrectableECCErrorCount"},
                                     {"ceCount", "/CorrectableECCErrorCount"}};

/* Map for OperatingConfig pdi to redfish string based on metric name*/
static MetricNameMap operatingConfigMap = {
    {"Utilization", "/BandwidthPercent"},
    {"OperatingSpeed", "/OperatingSpeedMHz"},
    {"MaxSpeed", "/MaxSpeedMHz"},
    {"MinSpeed", "/MinSpeedMHz"},
    {"SpeedLimit", "/SpeedLimitMHz"},
    {"SpeedLocked", "/SpeedLocked"}};

/* Map for DIMM pdi to redfish string based on metric name*/
static MetricNameMap dimmMap = {
    {"MemoryConfiguredSpeedInMhz", "/OperatingSpeedMHz"},
    {"Utilization", "/BandwidthPercent"}};

/* Map for PCIe device pdi to redfish string based on metric name*/
static MetricNameMap pcieDeviceMap = {
    {"PCIeType", "#/PCIeInterface/PCIeType"},
    {"MaxPCIeType", "#/PCIeInterface/MaxPCIeType"},
    {"LanesInUse", "#/PCIeInterface/LanesInUse"},
    {"MaxLanes", "#/PCIeInterface/MaxLanes"}};

/* Map for MemoryRowRemapping pdi to redfish string based on metric name*/
static MetricNameMap memoryRowRemappingMap = {
    {"ueRowRemappingCount",
     "/Oem/Nvidia/RowRemapping/UncorrectableRowRemappingCount"},
    {"ceRowRemappingCount",
     "/Oem/Nvidia/RowRemapping/CorrectableRowRemappingCount"},
    {"RowRemappingFailureState", "/Oem/Nvidia/RowRemappingFailed"},
    {"MaxRemappingAvailablityBankCount",
     "/Oem/Nvidia/RowRemapping/MaxAvailablityBankCount"},
    {"HighRemappingAvailablityBankCount",
     "/Oem/Nvidia/RowRemapping/HighAvailablityBankCount"},
    {"PartialRemappingAvailablityBankCount",
     "/Oem/Nvidia/RowRemapping/PartialAvailablityBankCount"},
    {"LowRemappingAvailablityBankCount",
     "/Oem/Nvidia/RowRemapping/LowAvailablityBankCount"},
    {"NoRemappingAvailablityBankCount",
     "/Oem/Nvidia/RowRemapping/NoAvailablityBankCount"}};

static MetricNameMap capacityUtilizationPercentMap{
    {"CapacityUtilizationPercent", "/CapacityUtilizationPercent"}};

/* Map for SMUtilization pdi to redfish string based on metric name*/
static MetricNameMap SMUtilizationPercentMap{
    {"SMUtilization", "/Oem/Nvidia/SMUtilizationPercent"}};

/* Map for OperationalStatus pdi to redfish string based on metric name*/
static MetricNameMap operationalStatusMap = {{"State", "/Status/State"}};

/* Map for MemorySpareChannel pdi to redfish string based on metric name*/
static MetricNameMap memorySpareChannelMap = {
    {"MemorySpareChannelPresence", "#/Oem/Nvidia/MemorySpareChannelPresence"}};

/* Map for EDPViolationState pdi to redfish string based on metric name*/
static MetricNameMap edpViolationStateMap = {
    {"Status", "#/Oem/Nvidia/EDPViolationState"}};

/* Map for Switch pdi to redfish string based on metric name*/
static MetricNameMap switchInterfaceMap = {
    {"CurrentBandwidth", "/CurrentBandwidthGbps"},
    {"MaxBandwidth", "/MaxBandwidthGbps"}};

/* Map for Power Limit pdi to redfish string based on metric name*/
static MetricNameMap PowerLimitMap = {{"MaxPowerWatts", "/MaxPowerWatts"},
                                      {"MinPowerWatts", "/MinPowerWatts"}};

/* Map for PCIe Reference Clock pdi to redfish string based on metric name*/
static MetricNameMap pcieRefClockMap = {
    {"PCIeReferenceClockEnabled", "#/Oem/Nvidia/PCIeReferenceClockEnabled"}};

/* Map for Port width pdi to redfish string based on metric name*/
static MetricNameMap portWidthInterfaceMap = {{"ActiveWidth", "#/ActiveWidth"}};

/* Map for Health to redfish string based on metric name*/
static MetricNameMap healthInterfaceMap = {{"Health", "#/Status/Health"}};

/* Map for HealthRollup to redfish string based on metric name*/
static MetricNameMap healthRollupInterfaceMap = {
    {"HealthRollup", "#/Status/HealthRollup"}};

/* This map is for PDI name to metric name. Key is pdi name and value is
 * corresponding metric name map */
static PDINameMap pdiNameMap = {
    {"xyz.openbmc_project.Inventory.Decorator.PortInfo", portInfoInterfaceMap},
    {"xyz.openbmc_project.Inventory.Decorator.PortState",
     portStateInterfaceMap},
    {"xyz.openbmc_project.Metrics.IBPort", ibPortInterfaceMap},
    {"xyz.openbmc_project.Metrics.PortMetricsOem1",
     portMetricsOem1InterfaceMap},
    {"xyz.openbmc_project.Metrics.PortMetricsOem2",
     portMetricsOem2InterfaceMap},
    {"xyz.openbmc_project.Metrics.PortMetricsOem3",
     portMetricsOem3InterfaceMap},
    {"xyz.openbmc_project.State.ProcessorPerformance", processorPerfMap},
    {"com.nvidia.NVLink.NVLinkMetrics", nvLinkMetricsMap},
    {"com.nvidia.GPMMetrics", gpmMetricsMap},
    {"xyz.openbmc_project.PCIe.PCIeECC", pcieECCMap},
    {"xyz.openbmc_project.Inventory.Item.Dimm.MemoryMetrics",
     capacityUtilizationPercentMap},
    {"xyz.openbmc_project.Memory.MemoryECC", memoryECCMap},
    {"xyz.openbmc_project.Inventory.Item.Cpu.OperatingConfig",
     operatingConfigMap},
    {"com.nvidia.SMUtilization", SMUtilizationPercentMap},
    {"xyz.openbmc_project.Inventory.Decorator.PowerLimit", PowerLimitMap},
    {"xyz.openbmc_project.Inventory.Item.Dimm", dimmMap},
    {"xyz.openbmc_project.Inventory.Item.PCIeDevice", pcieDeviceMap},
    {"xyz.openbmc_project.Inventory.Item.Switch", switchInterfaceMap},
    {"xyz.openbmc_project.State.Decorator.OperationalStatus",
     operationalStatusMap},
    {"com.nvidia.MemoryRowRemapping", memoryRowRemappingMap},
    {"com.nvidia.MemorySpareChannel", memorySpareChannelMap},
    {"xyz.openbmc_project.State.Decorator.PowerSystemInputs",
     edpViolationStateMap},
    {"xyz.openbmc_project.Inventory.Decorator.PortWidth",
     portWidthInterfaceMap},
    {"xyz.openbmc_project.Inventory.Decorator.PCIeRefClock", pcieRefClockMap},
    {"xyz.openbmc_project.State.Decorator.Health", healthInterfaceMap},
    {"xyz.openbmc_project.State.Decorator.HealthRollup",
     healthRollupInterfaceMap}};

/**
 * @brief This method will form suffix for redfish URI for device/sub device
 * property.
 *
 * @param[in] ifaceName - pdi name
 * @param[in] metricName - metric name
 * @return string
 */
inline string getPropertySuffix(const string& ifaceName,
                                const string& metricName)
{
    string suffix;
    if (pdiNameMap.find(ifaceName) != pdiNameMap.end())
    {
        if (pdiNameMap[ifaceName].find(metricName) !=
            pdiNameMap[ifaceName].end())
        {
            return pdiNameMap[ifaceName][metricName];
        }
    }
    return suffix;
}

/**
 * @brief Method to get reason type of the metric.
 *
 * @param[in] reason
 * @return string
 */
inline string toReasonType(const string& reason)
{
    if (reasonTypeMap.find(reason) != reasonTypeMap.end())
    {
        return reasonTypeMap[reason];
    }
    return "";
}

/**
 * @brief Method to get pcie type of metric from PDI.
 *
 * @param[in] pcieType
 * @return string
 */
inline string toPCIeType(const string& pcieType)
{
    if (pcieTypeMap.find(pcieType) != pcieTypeMap.end())
    {
        return pcieTypeMap[pcieType];
    }
    // Unknown or others
    return "Unknown";
}

/**
 * @brief Method to get the Power State Type of metric from PDI .
 *
 * @param[in] stateType
 * @return string
 */
inline string getPowerStateType(const string& stateType)
{
    if (powerStateTypeMap.find(stateType) != powerStateTypeMap.end())
    {
        return powerStateTypeMap[stateType];
    }
    // Unknown or others
    return "";
}

/**
 * @brief Method to get the Processor and cpu number for the device name.
 *
 * @param[in] deviceName
 * @return pair<string, string>
 * e.g For the input "ProcessorModule_1_CPU_0_CoreUtil_64", it will
 * output (1, 0).
 * For the input "ProcessorModule_0_Vreg_0_SocVoltage_0", it will
 * output (0, 0).
 */

inline pair<string, string> getProcessorAndCpuNum(string deviceName)
{
    vector<string> deviceNameKeys;
    boost::trim_if(deviceName, boost::is_any_of("_"));
    boost::split(deviceNameKeys, deviceName, boost::is_any_of("_"));
    if ((deviceNameKeys[1].size() == 1 && isdigit(deviceNameKeys[1][0])) &&
        deviceNameKeys[3].size() == 1 && isdigit(deviceNameKeys[3][0]))
    {
        return make_pair(deviceNameKeys[1], deviceNameKeys[3]);
    }
    return {"", ""};
}

/**
 * @brief This method returns translated string for metric reading values based
 * on PDI and metric name.
 *
 * @param[in] ifaceName - pdi
 * @param metricName - metric name
 * @param[in] reading - metric reading value
 * @return string
 */
inline string translateReading(const string& ifaceName,
                               const string& metricName, const string& reading)
{
    string metricValue;
    if (ifaceName == "xyz.openbmc_project.State.ProcessorPerformance")
    {
        if (metricName == "ThrottleReason")
        {
            metricValue = toReasonType(reading);
        }
        else if (metricName == "Value")
        {
            metricValue = toReasonType(reading);
        }
    }
    else if (ifaceName == "xyz.openbmc_project.PCIe.PCIeECC")
    {
        if (metricName == "PCIeType")
        {
            metricValue = toPCIeType(reading);
        }
    }
    else if (ifaceName == "xyz.openbmc_project.Inventory.Decorator.PortState")
    {
        if (metricName == "LinkStatus")
        {
            metricValue = getLinkStatusType(reading);
        }
        if (metricName == "LinkState")
        {
            metricValue = getLinkStateType(reading);
        }
    }
    else if (ifaceName ==
             "xyz.openbmc_project.State.Decorator.PowerSystemInputs")
    {
        if (metricName == "Status")
        {
            metricValue = getPowerSystemInputType(reading);
        }
    }
    else if (ifaceName ==
             "xyz.openbmc_project.State.Decorator.OperationalStatus")
    {
        if (metricName == "State")
        {
            metricValue = getPowerStateType(reading);
        }
    }
    else
    {
        metricValue = reading;
    }
    return metricValue;
}

/**
 * @brief Method to generate metric property uri from namespace, devicename and
 * other properties.
 *
 * @param[in] deviceType
 * @param[in] deviceName
 * @param[in] subDeviceName
 * @param[in] devicePath
 * @param[in] metricName
 * @param[in] ifaceName
 * @return string
 */
inline string generateURI(const string& deviceType, const string& deviceName,
                          const string& subDeviceName, const string& devicePath,
                          const string& metricName, const string& ifaceName)
{
    string metricURI;
    string propSuffix;
    // form redfish URI for sub device
    if (deviceType == "PlatformEnvironmentMetrics")
    {
        metricURI = "/redfish/v1/Chassis/";
        metricURI += deviceName;
        metricURI += "/Sensors/";
        metricURI += subDeviceName;
    }
    else if (deviceType == "CpuProcessorMetrics")
    {
        if (ifaceName == "xyz.openbmc_project.Sensor.Value")
        {
            metricURI = "/redfish/v1/Chassis/" PLATFORMDEVICEPREFIX;
            metricURI += deviceName;
            metricURI += "/Sensors/";
            metricURI += subDeviceName;
        }
        else if (ifaceName == "com.nvidia.MemorySpareChannel" ||
                 ifaceName ==
                     "xyz.openbmc_project.State.Decorator.PowerSystemInputs" ||
                 ifaceName == "xyz.openbmc_project.State.ProcessorPerformance")
        {
            metricURI = "/redfish/v1/Systems/" PLATFORMSYSTEMID;
            metricURI += "/Processors/";
            metricURI += deviceName;
            metricURI += "/ProcessorMetrics";
            propSuffix = getPropertySuffix(ifaceName, metricName);
        }
        else
        {
            metricURI = "/redfish/v1/Systems/" PLATFORMSYSTEMID;
            metricURI += "/Processors/";
            metricURI += deviceName;
            metricURI += "/Ports/";
            metricURI += subDeviceName;
            propSuffix = getPropertySuffix(ifaceName, metricName);
        }
    }
    else if (deviceType == "ProcessorPortMetrics")
    {
        metricURI = "/redfish/v1/Systems/" PLATFORMSYSTEMID;
        metricURI += "/Processors/";
        metricURI += deviceName;
        metricURI += "/Ports/";
        metricURI += subDeviceName;
        propSuffix = getPropertySuffix(ifaceName, metricName);
    }
    else if (deviceType == "ProcessorPortGPMMetrics")
    {
        metricURI = "/redfish/v1/Systems/" PLATFORMSYSTEMID;
        metricURI += "/Processors/";
        metricURI += deviceName;
        metricURI += "/Ports/";
        metricURI += subDeviceName;
        metricURI += "/Metrics#";
        propSuffix = getPropertySuffix(ifaceName, metricName);
    }
    else if (deviceType == "NVSwitchPortMetrics")
    {
        metricURI = "/redfish/v1/Fabrics/" PLATFORMDEVICEPREFIX;
        metricURI += "NVLinkFabric_0/Switches/";
        metricURI += deviceName;
        metricURI += "/Ports/";
        metricURI += subDeviceName;
        propSuffix = getPropertySuffix(ifaceName, metricName);
    }
    else if (deviceType == "NetworkAdapterPortMetrics")
    {
        metricURI = "/redfish/v1/Chassis/" PLATFORMDEVICEPREFIX;
        metricURI += deviceName;
        metricURI += "/NetworkAdapters/";
        metricURI += deviceName;
        metricURI += "/Ports/";
        metricURI += subDeviceName;
        propSuffix = getPropertySuffix(ifaceName, metricName);
    }
    else if (deviceType == "ProcessorMetrics")
    {
        metricURI = "/redfish/v1/Systems/" PLATFORMSYSTEMID;
        metricURI += "/Processors/";
        metricURI += deviceName;
        metricURI += "/ProcessorMetrics#";
        if (ifaceName == "xyz.openbmc_project.Memory.MemoryECC")
        {
            metricURI += "/CacheMetricsTotal/LifeTime";
        }
        else if (ifaceName == "xyz.openbmc_project.PCIe.PCIeECC")
        {
            if (metricName == "PCIeType" || metricName == "MaxLanes" ||
                metricName == "LanesInUse")
            {
                sdbusplus::message::object_path deviceObjectPath(devicePath);
                const string childDeviceName = deviceObjectPath.filename();
                string parentDeviceName = PLATFORMDEVICEPREFIX;
                parentDeviceName += childDeviceName;
                metricURI = "/redfish/v1/Chassis/";
                metricURI += parentDeviceName;
                metricURI += "/PCIeDevices/";
                metricURI += childDeviceName;
            }
        }
        else if (ifaceName ==
                 "xyz.openbmc_project.State.Decorator.OperationalStatus")
        {
            metricURI = "/redfish/v1/Systems/" PLATFORMSYSTEMID;
            metricURI += "/Processors/";
            metricURI += deviceName;
            metricURI += "#";
        }
        else if (ifaceName ==
                 "xyz.openbmc_project.Inventory.Decorator.PowerLimit")
        {
            metricURI = "/redfish/v1/Chassis/" PLATFORMDEVICEPREFIX;
            metricURI += deviceName;
            metricURI += "#";
        }
        else if (ifaceName ==
                 "xyz.openbmc_project.Inventory.Item.Cpu.OperatingConfig")
        {
            if (metricName == "MaxSpeed" || metricName == "MinSpeed" ||
                metricName == "SpeedLimit" || metricName == "SpeedLocked")
            {
                metricURI = "/redfish/v1/Systems/" PLATFORMSYSTEMID;
                metricURI += "/Processors/";
                metricURI += deviceName;
                metricURI += "#";
            }
        }
        propSuffix = getPropertySuffix(ifaceName, metricName);
    }
    else if (deviceType == "ProcessorGPMMetrics")
    {
        metricURI = "/redfish/v1/Systems/" PLATFORMSYSTEMID;
        metricURI += "/Processors/";
        metricURI += deviceName;
        metricURI += "/ProcessorMetrics#";
        propSuffix = getPropertySuffix(ifaceName, metricName);
    }
    else if (deviceType == "NVSwitchMetrics")
    {
        metricURI = "/redfish/v1/Fabrics/" PLATFORMDEVICEPREFIX;
        metricURI += "NVLinkFabric_0/Switches/";
        metricURI += deviceName;
        if (!(metricName == "CurrentBandwidth" || metricName == "MaxBandwidth"))
        {
            metricURI += "/SwitchMetrics#";
        }
        else
        {
            metricURI += "#";
        }
        if (ifaceName == "xyz.openbmc_project.Memory.MemoryECC")
        {
            metricURI += "/InternalMemoryMetrics/LifeTime";
        }
        propSuffix = getPropertySuffix(ifaceName, metricName);
    }
    else if (deviceType == "PCIeRetimerMetrics")
    {
        metricURI = "/redfish/v1/Chassis/";
        metricURI += deviceName;
        metricURI += "/PCIeDevices/";
        metricURI += subDeviceName;
        propSuffix = getPropertySuffix(ifaceName, metricName);
    }
    else if (deviceType == "PCIeRetimerPortMetrics")
    {
        size_t pos = deviceName.rfind('_');
        std::string retimerID = "0";
        if (pos != std::string::npos)
        {
            retimerID = deviceName.substr(pos + 1);
        }

        metricURI = "/redfish/v1/Fabrics/" PLATFORMDEVICEPREFIX;
        metricURI += "PCIeRetimerTopology_" + retimerID;
        metricURI += "/Switches/";
        metricURI += deviceName;
        metricURI += "/Ports/";
        metricURI += subDeviceName;
        if (ifaceName == "xyz.openbmc_project.PCIe.PCIeECC")
        {
            metricURI += "/Metrics#";
        }

        propSuffix = getPropertySuffix(ifaceName, metricName);
    }
    else if (deviceType == "MemoryMetrics")
    {
        metricURI = "/redfish/v1/Systems/" PLATFORMSYSTEMID;
        metricURI += "/Memory/";
        metricURI += deviceName;
        if (ifaceName == "com.nvidia.MemoryRowRemapping")
        {
            if (metricName == "RowRemappingFailureState" ||
                metricName == "RowRemappingPendingState")
            {
                metricURI += "#";
            }
            else
            {
                metricURI += "/MemoryMetrics#";
            }
        }
        else if (ifaceName ==
                 "xyz.openbmc_project.Inventory.Item.Dimm.MemoryMetrics")
        {
            if (metricName == "CapacityUtilizationPercent")
            {
                metricURI += "/MemoryMetrics#";
            }
        }
        else if (ifaceName == "xyz.openbmc_project.Memory.MemoryECC")
        {
            metricURI += "/MemoryMetrics#/LifeTime";
        }
        else
        {
            metricURI += "/MemoryMetrics#";
        }
        propSuffix = getPropertySuffix(ifaceName, metricName);
    }
    else if (deviceType == "HealthMetrics")
    {
        metricURI = "/redfish/v1/Chassis/" PLATFORMDEVICEPREFIX;
        std::string systemdId = PLATFORMDEVICEPREFIX + deviceName;
        if (systemdId == PLATFORMSYSTEMID)
        {
            metricURI = "/redfish/v1/Systems/" PLATFORMDEVICEPREFIX;
        }
        metricURI += deviceName;
        propSuffix = getPropertySuffix(ifaceName, metricName);
    }
    else
    {
        metricURI.clear();
    }

    if (!propSuffix.empty())
    {
        metricURI += propSuffix;
    }
    else
    {
        if (!((deviceType != "PlatformEnvironmentMetrics") ||
              (deviceType != "CpuProcessorMetrics")))
        {
            metricURI.clear();
        }
    }
    return metricURI;
}

/**
 * @brief Method to translate D-Bus throttle reason to redfish ThrottleDuration
 *
 * @param[in] metricName
 * @param[in] reading
 * @return string
 */
inline string translateThrottleDuration(const string& metricName,
                                        const uint64_t& reading)
{
    string metricValue;
    if ((metricName == "PowerLimitThrottleDuration") ||
        (metricName == "ThermalLimitThrottleDuration") ||
        (metricName == "HardwareViolationThrottleDuration") ||
        (metricName == "GlobalSoftwareViolationThrottleDuration"))
    {
        optional<string> duration =
            nv::sensor_aggregation::metricUtils::toDurationStringFromNano(
                reading);

        if (duration)
        {
            metricValue = *duration;
        }
    }
    else
    {
        metricValue = to_string(reading);
    }
    return metricValue;
}
/**
 * @brief Method to translate D-Bus AccumlatedDuration redfish AccumlatedDuratio
 *
 * @param reading
 * @return string
 */
inline string translateAccumlatedDuration(const uint64_t& reading)
{
    std::string metricValue;
    std::optional<std::string> duration =
        nv::sensor_aggregation::metricUtils::toDurationStringFromUint(reading);
    if (duration)
    {
        metricValue = *duration;
    }

    return metricValue;
}

/**
 * @brief This method returns metric values for each of namespaces and device
 * name for simple and array data types. Ouput will map of be a shared memory
 * key and value. Value contains metric property, translated value and
 * timestamp. This method should be used during discovery or for array value
 * updates.
 *
 * @param[in] deviceType
 * @param[in] deviceName
 * @param[in] subDeviceName
 * @param[in] devicePath
 * @param[in] metricName
 * @param[in] ifaceName
 * @param[in] value
 * @return pair<unordered_map<SHMKey, SHMValue>, bool>
 */
inline pair<unordered_map<SHMKey, SHMValue>, bool>
    getMetricValues(const string& deviceType, const string& deviceName,
                    const string& subDeviceName, const string& devicePath,
                    const string& metricName, const string& ifaceName,
                    DbusVariantType& value)
{
    unordered_map<SHMKey, SHMValue> shmValues;
    bool isList = false;
    if (const vector<string>* readingArray = get_if<vector<string>>(&value))
    {
        // This is for the property whose value is of type list and each element
        // in the list on the redfish is represented with
        // "PropertyName/<index_of_list_element>". and it always starts with 0
        // Eg:- ThrottleReasosns: [Idle, AppClock]-> "Idle" maps to
        // ThrottleReasons/0
        isList = true;
        int i = 0;
        for (const string& reading : *readingArray)
        {
            string val = translateReading(ifaceName, metricName, reading);
            string metricProp = generateURI(deviceType, deviceName,
                                            subDeviceName, devicePath,
                                            metricName, ifaceName);
            metricProp += "/";
            metricProp += to_string(i);
            string sensorKey = devicePath + "/" + ifaceName + "." + metricName +
                               "/" + to_string(i);
            SHMValue shmValue = {metricProp, val};
            shmValues.emplace(sensorKey, shmValue);
            i++;
        }
    }
    else if (const vector<double>* readingArray =
                 get_if<vector<double>>(&value))
    {
        // This is for the property whose value is of type list and each element
        // in the list on the redfish is represented with
        // "PropertyName/<index_of_list_element>". and it always starts with 0
        isList = true;
        int i = 0;
        for (const double& reading : *readingArray)
        {
            string val = to_string(reading);
            string metricProp = generateURI(deviceType, deviceName,
                                            subDeviceName, devicePath,
                                            metricName, ifaceName);
            metricProp += "/";
            metricProp += to_string(i);
            string sensorKey = devicePath + "/" + ifaceName + "." + metricName +
                               "/" + to_string(i);
            SHMValue shmValue = {metricProp, val};
            shmValues.emplace(sensorKey, shmValue);
            i++;
        }
    }
    else
    {
        const string metricProp = generateURI(deviceType, deviceName,
                                              subDeviceName, devicePath,
                                              metricName, ifaceName);
        if (metricProp.empty())
        {
            string errorMessage =
                "SHMEMDEBUG: Metric Property Empty for deviceType " +
                deviceType + " deviceName " + deviceName + " subDeviceName " +
                subDeviceName + " devicePath " + devicePath + " metricName " +
                metricName + " ifaceName " + ifaceName;
            LOG_ERROR(errorMessage);
            return {shmValues, isList};
        }
        string val;
        if (const string* reading = get_if<string>(&value))
        {
            val = translateReading(ifaceName, metricName, *reading);
        }
        else if (const int* reading = get_if<int>(&value))
        {
            val = to_string(*reading);
        }
        else if (const int16_t* reading = get_if<int16_t>(&value))
        {
            val = to_string(*reading);
        }
        else if (const int64_t* reading = get_if<int64_t>(&value))
        {
            val = to_string(*reading);
        }
        else if (const uint16_t* reading = get_if<uint16_t>(&value))
        {
            val = to_string(*reading);
        }
        else if (const uint32_t* reading = get_if<uint32_t>(&value))
        {
            val = to_string(*reading);
        }
        else if (const uint64_t* reading = get_if<uint64_t>(&value))
        {
            if ((ifaceName ==
                 "xyz.openbmc_project.State.ProcessorPerformance") &&
                ((metricName == "AccumulatedSMUtilizationDuration") ||
                 (metricName == "AccumulatedGPUContextUtilizationDuration")))
            {
                val = translateAccumlatedDuration(*reading);
            }
            else
            {
                val = translateThrottleDuration(metricName, *reading);
            }
        }
        else if (const double* reading = get_if<double>(&value))
        {
            val = to_string(*reading);
        }
        else if (const bool* reading = get_if<bool>(&value))
        {
            val = "false";
            if (*reading == true)
            {
                val = "true";
            }
        }
        string sensorKey = devicePath + "/" + ifaceName + "." + metricName;
        SHMValue shmValue = {metricProp, val};
        shmValues.emplace(sensorKey, shmValue);
    }
    return {shmValues, isList};
}

/**
 * @brief This method returns metric value for namespaces and device
 * name for simple and array data types. Ouput will be a shared memory key and
 * value. Value contains metric property, translated value and timestamp. This
 * method should be used during value and timestamp and nan updates.
 *
 * @param[in] metricName
 * @param[in] ifaceName
 * @param[in] value
 * @return SHMValue
 */
inline SHMValue getMetricValue(const string& metricName,
                               const string& ifaceName, DbusVariantType& value)
{
    string val;
    if (const string* reading = get_if<string>(&value))
    {
        val = translateReading(ifaceName, metricName, *reading);
    }
    else if (const int* reading = get_if<int>(&value))
    {
        val = to_string(*reading);
    }
    else if (const int16_t* reading = get_if<int16_t>(&value))
    {
        val = to_string(*reading);
    }
    else if (const int64_t* reading = get_if<int64_t>(&value))
    {
        val = to_string(*reading);
    }
    else if (const uint16_t* reading = get_if<uint16_t>(&value))
    {
        val = to_string(*reading);
    }
    else if (const uint32_t* reading = get_if<uint32_t>(&value))
    {
        val = to_string(*reading);
    }
    else if (const uint64_t* reading = get_if<uint64_t>(&value))
    {
        if ((ifaceName == "xyz.openbmc_project.State.ProcessorPerformance") &&
            ((metricName == "AccumulatedSMUtilizationDuration") ||
             (metricName == "AccumulatedGPUContextUtilizationDuration")))
        {
            val = translateAccumlatedDuration(*reading);
        }
        else
        {
            val = translateThrottleDuration(metricName, *reading);
        }
    }
    else if (const double* reading = get_if<double>(&value))
    {
        val = to_string(*reading);
    }
    else if (const bool* reading = get_if<bool>(&value))
    {
        val = "false";
        if (*reading == true)
        {
            val = "true";
        }
    }
    SHMValue shmValue = {"", val};
    return shmValue;
}

} // namespace metricUtils
} // namespace sensor_aggregation
} // namespace nv
