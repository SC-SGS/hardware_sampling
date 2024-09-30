/**
 * @file
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Implements utility functionality for the CPU sampler.
 */

#ifndef HWS_CPU_UTILITY_HPP_
#define HWS_CPU_UTILITY_HPP_
#pragma once

#include "fmt/format.h"  // fmt::format

#include <stdexcept>    // std::runtime_error
#include <string>       // std::string
#include <string_view>  // std::string_view

namespace hws::detail {

/**
 * @def HWS_SUBPROCESS_ERROR_CHECK
 * @brief Defines the `HWS_SUBPROCESS_ERROR_CHECK` macro if `HWS_ERROR_CHECKS_ENABLED` is defined, does nothing otherwise.
 * @details Throws an exception if a subprocess call returns with an error.
 */
#if defined(HWS_ERROR_CHECKS_ENABLED)
    #define HWS_SUBPROCESS_ERROR_CHECK(subprocess_func)                                                                \
        {                                                                                                              \
            const int errc = subprocess_func;                                                                          \
            if (errc != 0) {                                                                                           \
                throw std::runtime_error{ fmt::format("Error calling subprocess function \"{}\"", #subprocess_func) }; \
            }                                                                                                          \
        }
#else
    #define HWS_SUBPROCESS_ERROR_CHECK(subprocess_func) subprocess_func;
#endif

/**
 * @brief Run a subprocess executing @p cmd_line and returning the stdout and stderr string.
 * @param[in] cmd_line the command line to execute
 * @return the stdout and stderr content encountered during executing @p cmd_line (`[[nodiscard]]`)
 */
[[nodiscard]] std::string run_subprocess(std::string_view cmd_line);

}  // namespace hws::detail

#endif  // HWS_CPU_UTILITY_HPP_
