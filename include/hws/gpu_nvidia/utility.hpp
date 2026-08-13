/**
 * @file
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Implements utility functionality for the NVIDIA GPU sampler.
 */

#ifndef HWS_GPU_NVIDIA_UTILITY_HPP_
#define HWS_GPU_NVIDIA_UTILITY_HPP_
#pragma once

#include "cuda_runtime_api.h"  // CUDA runtime functions
#include "fmt/format.h"        // fmt::format
#include "nvml.h"              // NVML runtime functions

#include <stdexcept>  // std::runtime_error
#include <string>     // std::string
#include <vector>     // std::vector

#if defined(HWS_MPI_SUPPORT_ENABLED)
    #include "hws/visible_gpu_device.hpp"  // hws::detail::visible_gpu_device
#endif

namespace hws::detail {

/**
 * @def HWS_NVML_ERROR_CHECK
 * @brief Defines the `HWS_NVML_ERROR_CHECK` macro if `HWS_ERROR_CHECKS_ENABLED` is defined, does nothing otherwise.
 * @details Throws an exception if an NVML call returns with an error. Additionally outputs a more concrete error string.
 */
/**
 * @def HWS_CUDA_ERROR_CHECK
 * @brief Defines the `HWS_CUDA_ERROR_CHECK` macro if `HWS_ERROR_CHECKS_ENABLED` is defined, does nothing otherwise.
 * @details Throws an exception if a CUDA call returns with an error. Additionally outputs a more concrete error string.
 */
#if defined(HWS_ERROR_CHECKS_ENABLED)
    #define HWS_NVML_ERROR_CHECK(nvml_func)                                                                                                                        \
        {                                                                                                                                                          \
            const nvmlReturn_t errc = nvml_func;                                                                                                                   \
            if (errc != NVML_SUCCESS) {                                                                                                                            \
                throw std::runtime_error{ fmt::format("Error in NVML function call \"{}\": {} ({})", #nvml_func, nvmlErrorString(errc), static_cast<int>(errc)) }; \
            }                                                                                                                                                      \
        }

    #define HWS_CUDA_ERROR_CHECK(cuda_func)                                                                                                                           \
        {                                                                                                                                                             \
            const cudaError_t errc = cuda_func;                                                                                                                       \
            if (errc != cudaSuccess) {                                                                                                                                \
                throw std::runtime_error{ fmt::format("Error in CUDA function call \"{}\": {} ({})", #cuda_func, cudaGetErrorName(errc), cudaGetErrorString(errc)) }; \
            }                                                                                                                                                         \
        }
#else
    #define HWS_NVML_ERROR_CHECK(nvml_func) nvml_func;
    #define HWS_CUDA_ERROR_CHECK(cuda_func) cuda_func;
#endif

#if CUDA_VERSION >= 12000

/**
 * @brief Convert the clock throttle reason event bitmask to a string representation. If the provided bitmask represents multiple reasons, they are split using "|".
 * @param[in] clocks_event_reasons the bitmask to convert to a string
 * @return all event throttle reasons (`[[nodiscard]]`)
 */
[[nodiscard]] std::string throttle_event_reason_to_string(unsigned long long clocks_event_reasons);

#endif

/**
 * @brief Return the PCI bus ID (e.g. `"0000:c1:00.0"`) of the NVIDIA GPU device with the given CUDA @p local_index.
 * @details This is the same, stable identifier used by `enumerate_all_nvidia_gpu_pci_bus_ids()`, so the two can be
 *          matched against each other to locate a CUDA-visible device within the full physical GPU topology.
 * @param[in] local_index the local CUDA device index
 * @return the PCI bus ID (`[[nodiscard]]`)
 */
[[nodiscard]] std::string nvidia_device_pci_bus_id(int local_index);

/**
 * @brief Enumerate the PCI bus IDs of every NVIDIA GPU physically present on the node, independent of any
 *        process-level device visibility filtering (e.g. `CUDA_VISIBLE_DEVICES`).
 * @details Reads the kernel's view of devices bound to the `nvidia` driver directly from
 *          `/sys/bus/pci/drivers/nvidia/`, since that sysfs directory - unlike the CUDA runtime's device
 *          enumeration - isn't affected by per-process visible-device environment variables. It *can* still be
 *          restricted below the true physical device count in a batch job with kernel-level (cgroup) device
 *          isolation for a partial-node allocation, so callers must treat an unexpectedly low count as "topology
 *          unknown", not as ground truth. Untested against real hardware (no NVIDIA GH200/EX254n access) - the
 *          `amdgpu` counterpart this mirrors (`hws::detail::enumerate_all_amd_gpu_pci_bus_ids()`) is; double-check
 *          the `nvidia` driver directory name and symlink layout on first use.
 * @return the sorted PCI bus IDs of all `nvidia`-bound devices, or an empty vector if the directory doesn't exist
 *         or isn't readable (`[[nodiscard]]`)
 */
[[nodiscard]] std::vector<std::string> enumerate_all_nvidia_gpu_pci_bus_ids();

#if defined(HWS_MPI_SUPPORT_ENABLED)

/**
 * @brief creates a list of all visible NVIDIA GPU devices
 *
 * @return a vector of all visible NVIDIA GPU devices on the local node, each with its local index and physical ID
 */
[[nodiscard]] std::vector<visible_gpu_device> enumerate_local_nvidia_devices();

#endif

}  // namespace hws::detail

#endif  // HWS_GPU_NVIDIA_UTILITY_HPP_
