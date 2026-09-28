/**
 * @file
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Implements utility functionality for the AMD GPU sampler.
 */

#ifndef HWS_GPU_AMD_UTILITY_HPP_
#define HWS_GPU_AMD_UTILITY_HPP_
#pragma once

#include "fmt/format.h"         // fmt::format
#include "rocm_smi/rocm_smi.h"  // ROCm SMI runtime functions

#include <stdexcept>  // std::runtime_error
#include <string>     // std::string
#include <vector>     // std::vector

#if defined(HWS_MPI_SUPPORT_ENABLED)
    #include "hws/visible_gpu_device.hpp"  // hws::detail::visible_gpu_device
#endif

namespace hws::detail {

/**
 * @def HWS_ROCM_SMI_ERROR_CHECK
 * @brief Defines the `HWS_ROCM_SMI_ERROR_CHECK` macro if `HWS_ERROR_CHECKS_ENABLED` is defined, does nothing otherwise.
 * @details Throws an exception if a ROCm SMI call returns with an error. Additionally outputs a more concrete error string if possible.
 */
/**
 * @def HWS_HIP_ERROR_CHECK
 * @brief Defines the `HWS_HIP_ERROR_CHECK` macro if `HWS_ERROR_CHECKS_ENABLED` is defined, does nothing otherwise.
 * @details Throws an exception if a HIP call returns with an error. Additionally outputs a more concrete error string.
 */
#if defined(HWS_ERROR_CHECKS_ENABLED)
    #define HWS_ROCM_SMI_ERROR_CHECK(rocm_smi_func)                                                                                                \
        {                                                                                                                                          \
            const rsmi_status_t errc = rocm_smi_func;                                                                                              \
            if (errc != RSMI_STATUS_SUCCESS) {                                                                                                     \
                const char *error_string;                                                                                                          \
                const rsmi_status_t ret = rsmi_status_string(errc, &error_string);                                                                 \
                if (ret == RSMI_STATUS_SUCCESS) {                                                                                                  \
                    throw std::runtime_error{ fmt::format("Error in ROCm SMI function call \"{}\": {}", #rocm_smi_func, error_string) };           \
                } else {                                                                                                                           \
                    throw std::runtime_error{ fmt::format("Error in ROCm SMI function call \"{}\": {}", #rocm_smi_func, static_cast<int>(errc)) }; \
                }                                                                                                                                  \
            }                                                                                                                                      \
        }

    #define HWS_HIP_ERROR_CHECK(hip_func)                                                                                             \
        {                                                                                                                             \
            const hipError_t errc = hip_func;                                                                                         \
            if (errc != hipSuccess) {                                                                                                 \
                throw std::runtime_error{ fmt::format("Error in HIP function call \"{}\": {}", #hip_func, hipGetErrorString(errc)) }; \
            }                                                                                                                         \
        }

#else
    #define HWS_ROCM_SMI_ERROR_CHECK(rocm_smi_func) rocm_smi_func;
    #define HWS_HIP_ERROR_CHECK(hip_func)                \
        {                                                \
            [[maybe_unused]] hipError_t errc = hip_func; \
        }
#endif

/**
 * @brief Convert the performance level value (`rsmi_dev_perf_level_t`) to a string.
 * @param[in] perf_level the bitmask to convert to a string
 * @return all event throttle reasons (`[[nodiscard]]`)
 */
[[nodiscard]] std::string performance_level_to_string(rsmi_dev_perf_level_t perf_level);

/**
 * @brief Return the PCI bus ID (e.g. `"0000:c1:00.0"`) of the AMD GPU device with the given HIP @p local_index.
 * @details This is the same, stable identifier used by `enumerate_all_amd_gpu_pci_bus_ids()`, so the two can be
 *          matched against each other to locate a HIP-visible device within the full physical GPU topology.
 * @param[in] local_index the local HIP device index
 * @return the PCI bus ID (`[[nodiscard]]`)
 */
[[nodiscard]] std::string amd_device_pci_bus_id(int local_index);

/**
 * @brief Enumerate the PCI bus IDs of every AMD GPU physically present on the node, independent of any
 *        process-level device visibility filtering (e.g. `HIP_VISIBLE_DEVICES`/`ROCR_VISIBLE_DEVICES`).
 * @details Reads the kernel's view of devices bound to the `amdgpu` driver directly from
 *          `/sys/bus/pci/drivers/amdgpu/`, since that sysfs directory - unlike the HIP runtime's device
 *          enumeration - isn't affected by per-process visible-device environment variables. It *can* still be
 *          restricted below the true physical device count in a batch job with kernel-level (cgroup) device
 *          isolation for a partial-node allocation, so callers must treat an unexpectedly low count as "topology
 *          unknown", not as ground truth.
 * @return the sorted PCI bus IDs of all `amdgpu`-bound devices, or an empty vector if the directory doesn't exist
 *         or isn't readable (`[[nodiscard]]`)
 */
[[nodiscard]] std::vector<std::string> enumerate_all_amd_gpu_pci_bus_ids();

#if defined(HWS_MPI_SUPPORT_ENABLED)

/**
 * @brief creates a list of all visible AMD GPU devices
 *
 * @return a vector of all visible AMD GPU devices on the local node, each with its local index and physical ID
 */
[[nodiscard]] std::vector<visible_gpu_device> enumerate_local_amd_devices();

#endif

}  // namespace hws::detail

#endif  // HWS_GPU_AMD_UTILITY_HPP_
