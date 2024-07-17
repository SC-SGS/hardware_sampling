/**
 * @file
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Implements utility functionality for the NVIDIA GPU sampler.
 */

#ifndef HARDWARE_SAMPLING_GPU_NVIDIA_UTILITY_HPP_
#define HARDWARE_SAMPLING_GPU_NVIDIA_UTILITY_HPP_
#pragma once

#include "nvml.h"  // NVML runtime functions

#include <format>     // std::format
#include <stdexcept>  // std::runtime_error

namespace hws::detail {

/**
 * @def HWS_NVML_ERROR_CHECK
 * @brief Defines the `HWS_NVML_ERROR_CHECK` macro if `HWS_ERROR_CHECKS_ENABLED` is defined, does nothing otherwise.
 * @details Throws an exception if an NVML call returns with an error. Additionally outputs a more concrete error string.
 */
#if defined(HWS_ERROR_CHECKS_ENABLED)
    #define HWS_NVML_ERROR_CHECK(nvml_func)                                                                                                                   \
        {                                                                                                                                                     \
            const nvmlReturn_t errc = nvml_func;                                                                                                              \
            if (errc != NVML_SUCCESS) {                                                                                                                       \
                throw runtime_error{ std::format("Error in NVML function call \"{}\": {} ({})", #nvml_func, nvmlErrorString(errc), static_cast<int>(errc)) }; \
            }                                                                                                                                                 \
        }
#else
    #define HWS_NVML_ERROR_CHECK(nvml_func) nvml_func;
#endif

}  // namespace hws::detail

#endif  // HARDWARE_SAMPLING_GPU_NVIDIA_UTILITY_HPP_
