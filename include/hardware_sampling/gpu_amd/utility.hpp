/**
 * @file
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Implements utility functionality for the AMD GPU sampler.
 */

#ifndef HARDWARE_SAMPLING_GPU_AMD_UTILITY_HPP_
#define HARDWARE_SAMPLING_GPU_AMD_UTILITY_HPP_
#pragma once

#include "rocm_smi/rocm_smi.h"  // ROCm SMI runtime functions

#include <format>     // std::format
#include <stdexcept>  // std::runtime_error

namespace hws {

/**
 * @def HWS_ROCM_SMI_ERROR_CHECK
 * @brief Defines the `HWS_ROCM_SMI_ERROR_CHECK` macro if `HWS_ERROR_CHECKS_ENABLED` is defined, does nothing otherwise.
 * @details Throws an exception if a ROCm SMI call returns with an error. Additionally outputs a more concrete error string if possible.
 */
#if defined(HWS_ERROR_CHECKS_ENABLED)
    #define HWS_ROCM_SMI_ERROR_CHECK(rocm_smi_func)                                                                                                \
        {                                                                                                                                          \
            const rsmi_status_t errc = rocm_smi_func;                                                                                              \
            if (errc != RSMI_STATUS_SUCCESS) {                                                                                                     \
                const char *error_string;                                                                                                          \
                const rsmi_status_t ret = rsmi_status_string(errc, &error_string);                                                                 \
                if (ret == RSMI_STATUS_SUCCESS) {                                                                                                  \
                    throw std::runtime_error{ std::format("Error in ROCm SMI function call \"{}\": {}", #rocm_smi_func, error_string) };           \
                } else {                                                                                                                           \
                    throw std::runtime_error{ std::format("Error in ROCm SMI function call \"{}\": {}", #rocm_smi_func, static_cast<int>(errc)) }; \
                }                                                                                                                                  \
            }                                                                                                                                      \
        }
#else
    #define HWS_ROCM_SMI_ERROR_CHECK(rocm_smi_func) rocm_smi_func;
#endif

}  // namespace hws

#endif  // HARDWARE_SAMPLING_GPU_AMD_UTILITY_HPP_
