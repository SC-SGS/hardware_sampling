/**
 * @file
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Implements utility functionality for the Intel GPU sampler.
 */

#ifndef HWS_GPU_INTEL_UTILITY_HPP_
#define HWS_GPU_INTEL_UTILITY_HPP_
#pragma once

#include "fmt/format.h"          // fmt::format
#include "level_zero/ze_api.h"   // Level Zero runtime functions
#include "level_zero/zes_api.h"  // Level Zero runtime functions

#include <stdexcept>    // std::runtime_error
#include <string>       // std::string
#include <string_view>  // std::string_view
#include <vector>       // std::vector

namespace hws::detail {

/**
 * @brief Given the Level Zero API error @p errc, returns a useful error string.
 * @param[in] errc the Level Zero API error code
 * @return the error string (`[[nodiscard]]`)
 */
[[nodiscard]] std::string_view to_result_string(ze_result_t errc);

/**
 * @def HWS_LEVEL_ZERO_ERROR_CHECK
 * @brief Defines the `HWS_LEVEL_ZERO_ERROR_CHECK` macro if `HWS_ERROR_CHECKS_ENABLED` is defined, does nothing otherwise.
 * @details Throws an exception if a Level Zero call returns with an error. Additionally outputs a more concrete custom error string.
 */
#if defined(HWS_ERROR_CHECKS_ENABLED)
    #define HWS_LEVEL_ZERO_ERROR_CHECK(level_zero_func)                                                                                            \
        {                                                                                                                                          \
            const ze_result_t errc = level_zero_func;                                                                                              \
            if (errc != ZE_RESULT_SUCCESS) {                                                                                                       \
                throw std::runtime_error{ fmt::format("Error in Level Zero function call \"{}\": {}", #level_zero_func, to_result_string(errc)) }; \
            }                                                                                                                                      \
        }
#else
    #define HWS_LEVEL_ZERO_ERROR_CHECK(level_zero_func) level_zero_func;
#endif

/**
 * @brief Convert the @p flags to a vector of strings.
 * @param[in] flags the flags to convert to strings
 * @return a vector containing all flags as strings (`[[nodiscard]]`)
 */
[[nodiscard]] std::vector<std::string> property_flags_to_vector(ze_device_property_flags_t flags);

/**
 * @brief Convert the throttle reason bitmask to a string representation. If the provided bitmask represents multiple reasons, they are split using "|".
 * @param[in] reasons the bitmask to convert to a string
 * @return all throttle reasons (`[[nodiscard]]`)
 */
[[nodiscard]] std::string throttle_reason_to_string(zes_freq_throttle_reason_flags_t reasons);

/**
 * @brief Convert a Level Zero memory type to a string representation.
 * @param[in] mem_type the Level Zero memory type
 * @return the string representation (`[[nodiscard]]`)
 */
[[nodiscard]] std::string memory_module_to_name(zes_mem_type_t mem_type);

/**
 * @brief Convert a Level Zero memory location to a string representation.
 * @param[in] mem_loc the Level Zero memory location
 * @return the string representation (`[[nodiscard]]`)
 */
[[nodiscard]] std::string memory_location_to_name(zes_mem_loc_t mem_loc);

}  // namespace hws::detail

#endif  // HWS_GPU_INTEL_UTILITY_HPP_
