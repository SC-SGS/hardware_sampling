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

#include <chrono>   // std::chrono::steady_clock::time_point
#include <cstddef>  // std::size_t
#include <format>   // std::formatter
#include <iosfwd>   // std::ostream forward declaration
#include <string>   // std::string
#include <vector>   // std::vector

namespace hws {

/**
 * @brief A class encapsulating all events.
 */
class events {
  public:
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
     * @brief Add a new event.
     * @param e the event
     */
    void add_event(event e);

    /**
     * @brief Add a new event.
     * @param[in] time_point the time point when the event occurred
     * @param[in] name the name of the event
     */
    void add_event(decltype(event::time_point) time_point, decltype(event::name) name);

    /**
     * @brief Return the event at index @p idx.
     * @param[in] idx the index of the event to retrieve
     * @return the event (`[[nodiscard]]`)
     */
    [[nodiscard]] event operator[](std::size_t idx) const noexcept;

    /**
     * @brief Return the event at index @p idx.
     * @param[in] idx the index of the event to retrieve
     * @throws std::out_of_range if the requested index is out-of-bounce
     * @return the event (`[[nodiscard]]`)
     */
    [[nodiscard]] event at(std::size_t idx) const;

    /**
     * @brief Return the number of recorded events.
     * @return the number of events (`[[nodiscard]]`)
     */
    [[nodiscard]] std::size_t num_events() const noexcept { return time_points_.size(); }

    /**
     * @brief Check whether any event has been recorded yet.
     * @return `true` if no event has been recorded, `false` otherwise (`[[nodiscard]]`)
     */
    [[nodiscard]] bool empty() const noexcept { return time_points_.empty(); }

    /**
     * @brief Get all time points.
     * @return the time points (`[[nodiscard]]`)
     */
    [[nodiscard]] const auto &get_time_points() const noexcept { return time_points_; }

    /**
     * @brief Get all event names.
     * @return the event names (`[[nodiscard]]`)
     */
    [[nodiscard]] const auto &get_names() const noexcept { return names_; }

  private:
    /// All time points at which an event occurred.
    std::vector<decltype(event::time_point)> time_points_;
    /// The names of the respective events.
    std::vector<decltype(event::name)> names_;
};

/**
 * @brief Output the event @p e to the given output-stream @p out.
 * @param[in,out] out the output-stream to write the event to
 * @param[in] e the event
 * @return the output-stream
 */
std::ostream &operator<<(std::ostream &out, const events::event &e);

/**
 * @brief Output all events @p e to the given output-stream @p out.
 * @param[in,out] out the output-stream to write all events
 * @param[in] e all events
 * @return the output-stream
 */
std::ostream &operator<<(std::ostream &out, const events &e);

}  // namespace hws

template <>
struct std::formatter<hws::events::event> : hws::detail::ostream_formatter { };

template <>
struct std::formatter<hws::events> : hws::detail::ostream_formatter { };

#endif  // HARDWARE_SAMPLING_EVENT_HPP_
