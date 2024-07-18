/**
 * @file
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Defines an event type.
 */

#ifndef HARDWARE_SAMPLING_EVENT_HPP_
#define HARDWARE_SAMPLING_EVENT_HPP_
#pragma once

#include "hardware_sampling/utility.hpp"  // hws::detail::ostream_formatter

#include <chrono>  // std::chrono::steady_clock::time_point
#include <format>  // std::formatter
#include <iosfwd>  // std::ostream forward declaration
#include <string>  // std::string

namespace hws {

/**
 * @brief A struct encapsulating a single event.
 */
struct event {
    /// The time point this event occurred at.
    std::chrono::steady_clock::time_point time_point;

    /// The name of this event.
    std::string name;
};

/**
 * @brief Output the event @p e to the given output-stream @p out.
 * @param[in,out] out the output-stream to write the event to
 * @param[in] e the event
 * @return the output-stream
 */
std::ostream &operator<<(std::ostream &out, const event &e);

}  // namespace hws

template <>
struct std::formatter<hws::event> : hws::detail::ostream_formatter { };

#endif  // HARDWARE_SAMPLING_EVENT_HPP_
