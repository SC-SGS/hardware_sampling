/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hws/gpu_nvidia/utility.hpp"

#include "fmt/format.h"  // fmt::format
#include "fmt/ranges.h"  // fmt::join
#include "nvml.h"        // NVML runtime functions

#include <string>  // std::string
#include <vector>  // std::vector

#if defined(HWS_MPI_SUPPORT_ENABLED) && defined(HWS_FOR_NVIDIA_GPUS_ENABLED)
    #include "hws/visible_gpu_device.hpp"  // hws::detail::visible_gpu_device, hws::detail::device_backend_kind
#endif

namespace hws::detail {

#if CUDA_VERSION >= 12000

std::string throttle_event_reason_to_string(const unsigned long long clocks_event_reasons) {
    if (clocks_event_reasons == 0ull) {
        return "None";
    } else {
        std::vector<std::string> reasons{};
        if ((clocks_event_reasons & nvmlClocksEventReasonApplicationsClocksSetting) != 0ull) {
            reasons.emplace_back("ApplicationsClocksSetting");
        }
        if ((clocks_event_reasons & nvmlClocksEventReasonDisplayClockSetting) != 0ull) {
            reasons.emplace_back("DisplayClockSetting");
        }
        if ((clocks_event_reasons & nvmlClocksEventReasonGpuIdle) != 0ull) {
            reasons.emplace_back("GpuIdle");
        }
        if ((clocks_event_reasons & nvmlClocksEventReasonSwPowerCap) != 0ull) {
            reasons.emplace_back("SwPowerCap");
        }
        if ((clocks_event_reasons & nvmlClocksEventReasonSwThermalSlowdown) != 0ull) {
            reasons.emplace_back("SwThermalSlowdown");
        }
        if ((clocks_event_reasons & nvmlClocksEventReasonSyncBoost) != 0ull) {
            reasons.emplace_back("SyncBoost");
        }
        if ((clocks_event_reasons & nvmlClocksThrottleReasonHwPowerBrakeSlowdown) != 0ull) {
            reasons.emplace_back("HwPowerBrakeSlowdown");
        }
        if ((clocks_event_reasons & nvmlClocksThrottleReasonHwSlowdown) != 0ull) {
            reasons.emplace_back("HwSlowdown");
        }
        if ((clocks_event_reasons & nvmlClocksThrottleReasonHwThermalSlowdown) != 0ull) {
            reasons.emplace_back("HwThermalSlowdown");
        }
        return fmt::format("{}", fmt::join(reasons, "|"));
    }
}

#endif

std::string nvidia_device_pci_bus_id(const int local_index) {
    char bus_id[64] = {};
    HWS_CUDA_ERROR_CHECK(cudaDeviceGetPCIBusId(bus_id, sizeof(bus_id), local_index));
    return std::string{ bus_id };
}

#if defined(HWS_MPI_SUPPORT_ENABLED) && defined(HWS_FOR_NVIDIA_GPUS_ENABLED)

namespace {

/**
 * @brief returns a stable physical ID for the NVIDIA GPU device with the given local index
 * The ID is at least unique per node and can be used to identify the same device across different MPI ranks on the same node.
 *
 * @param local_index the local index of the NVIDIA GPU device
 * @return the physical ID of the NVIDIA GPU device
 */
[[nodiscard]] std::string nvidia_physical_id(const int local_index) {
    return std::string{ "nvidia:" } + nvidia_device_pci_bus_id(local_index);
}

}  // namespace

[[nodiscard]] std::vector<visible_gpu_device> enumerate_local_nvidia_devices() {
    std::vector<visible_gpu_device> out;
    int count = 0;
    HWS_CUDA_ERROR_CHECK(cudaGetDeviceCount(&count));
    for (int i = 0; i < count; ++i) {
        visible_gpu_device d;
        d.backend = device_backend_kind::nvidia;
        d.local_index = i;
        d.physical_id = nvidia_physical_id(i);
        out.push_back(std::move(d));
    }
    return out;
}

#endif

}  // namespace hws::detail
