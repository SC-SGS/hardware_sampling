/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hws/gpu_amd/utility.hpp"

#include "rocm_smi/rocm_smi.h"  // ROCm SMI runtime functions

#include <string>  // std::string
#include <vector>  // std::vector

#if defined(HWS_MPI_SUPPORT_ENABLED) && defined(HWS_FOR_AMD_GPUS_ENABLED)
    #include "hws/visible_gpu_device.hpp"  // hws::detail::visible_gpu_device, hws::detail::device_backend_kind

    #include "hip/hip_runtime_api.h"  // hipGetDeviceCount, hipDeviceGetPCIBusId
#endif

namespace hws::detail {

std::string performance_level_to_string(const rsmi_dev_perf_level_t perf_level) {
    switch (perf_level) {
        case RSMI_DEV_PERF_LEVEL_AUTO:
            return "auto";
        case RSMI_DEV_PERF_LEVEL_LOW:
            return "low";
        case RSMI_DEV_PERF_LEVEL_HIGH:
            return "high";
        case RSMI_DEV_PERF_LEVEL_MANUAL:
            return "manual";
        case RSMI_DEV_PERF_LEVEL_STABLE_STD:
            return "stable_std";
        case RSMI_DEV_PERF_LEVEL_STABLE_PEAK:
            return "stable_peak";
        case RSMI_DEV_PERF_LEVEL_STABLE_MIN_MCLK:
            return "stable_min_mclk";
        case RSMI_DEV_PERF_LEVEL_STABLE_MIN_SCLK:
            return "stable_min_sclk";
        case RSMI_DEV_PERF_LEVEL_DETERMINISM:
            return "determinism";
        case RSMI_DEV_PERF_LEVEL_UNKNOWN:
        default:
            return "unknown";
    }
}

#if defined(HWS_MPI_SUPPORT_ENABLED) && defined(HWS_FOR_AMD_GPUS_ENABLED)

namespace {

/**
 * @brief returns a stable physical ID for the AMD GPU device with the given local index
 * The ID is at least unique per node and can be used to identify the same device across different MPI ranks on the same node.
 *
 * @param local_index the local index of the AMD GPU device
 * @return the physical ID of the AMD GPU device
 */
[[nodiscard]] std::string amd_physical_id(const int local_index) {
    char bus_id[64] = {};
    HWS_HIP_ERROR_CHECK(hipDeviceGetPCIBusId(bus_id, sizeof(bus_id), local_index));
    return std::string{ "amd:" } + bus_id;
}

}  // namespace

[[nodiscard]] std::vector<visible_gpu_device> enumerate_local_amd_devices() {
    std::vector<visible_gpu_device> out;
    int count = 0;
    HWS_HIP_ERROR_CHECK(hipGetDeviceCount(&count));
    for (int i = 0; i < count; ++i) {
        visible_gpu_device d;
        d.backend = device_backend_kind::amd;
        d.local_index = i;
        d.physical_id = amd_physical_id(i);
        out.push_back(std::move(d));
    }
    return out;
}

#endif

}  // namespace hws::detail
