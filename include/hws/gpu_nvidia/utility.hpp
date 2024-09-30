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

/**
 * @brief Convert the clock throttle reason event bitmask to a string representation. If the provided bitmask represents multiple reasons, they are split using "|".
 * @param[in] clocks_event_reasons the bitmask to convert to a string
 * @return all event throttle reasons (`[[nodiscard]]`)
 */
[[nodiscard]] std::string throttle_event_reason_to_string(unsigned long long clocks_event_reasons);

}  // namespace hws::detail

#endif  // HWS_GPU_NVIDIA_UTILITY_HPP_
