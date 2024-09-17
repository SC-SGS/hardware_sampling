/**
 * @file
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Defines a struct encapsulating a single event with a relative time point.
 */

#ifndef HARDWARE_SAMPLING_BINDINGS_RELATIVE_EVENT_HPP_
#define HARDWARE_SAMPLING_BINDINGS_RELATIVE_EVENT_HPP_

#include <string>   // std::string
#include <utility>  // std::move

namespace hws::detail {

/**
 * @brief A struct encapsulating a single event with a relative time point.
 */
struct relative_event {
    /**
     * @brief Construct a new event given a time point and name.
     * @param[in] time_point_p the time when the event occurred relative to the first event
     * @param[in] name_p the name of the event
     */
    relative_event(const double relative_time_point_p, std::string name_p) :
        relative_time_point{ relative_time_point_p },
        name{ std::move(name_p) } { }

    /// The relative time point this event occurred at.
    double relative_time_point;
    /// The name of this event.
    std::string name;
};

}  // namespace hws::detail

#endif  // HARDWARE_SAMPLING_BINDINGS_RELATIVE_EVENT_HPP_
