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

#include "fmt/ostream.h"  // fmt::formatter, fmt::ostream_formatter

#include <chrono>   // std::chrono::steady_clock::time_point
#include <iosfwd>   // std::ostream forward declaration
#include <string>   // std::string
#include <utility>  // std::move

namespace hws {

/**
 * @brief A struct encapsulating a single event.
 */
struct event {
    /**
     * @brief Construct a new event given a time point and name.
     * @param[in] time_point_p the time when the event occurred
     * @param[in] name_p the name of the event
     */
    event(const std::chrono::steady_clock::time_point time_point_p, std::string name_p) :
        time_point{ time_point_p },
        name{ std::move(name_p) } { }

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

/// @cond Doxygen_suppress

template <>
struct fmt::formatter<hws::event> : fmt::ostream_formatter { };

/// @endcond

#endif  // HARDWARE_SAMPLING_EVENT_HPP_
