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

}  // namespace hws::detail

#endif  // HWS_GPU_AMD_UTILITY_HPP_
