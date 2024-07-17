/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hardware_sampling/events.hpp"

#include "hardware_sampling/utility.hpp"  // hws::detail::{join, durations_from_reference_time, time_points_to_epoch}

#include <chrono>     // std::chrono::steady_clock::time_point
#include <cstddef>    // std::size_t
#include <format>     // std::format
#include <ostream>    // std::ostream
#include <stdexcept>  // std::out_of_range
#include <string>     // std::string
#include <utility>    // std::move
#include <vector>     // std::vector

namespace hws {

void events::add_event(event e) {
    this->time_points_.push_back(std::move(e.time_point));
    this->names_.push_back(std::format("{}", std::move(e.name)));
}

void events::add_event(decltype(event::time_point) time_point, decltype(event::name) name) {
    this->time_points_.push_back(std::move(time_point));
    this->names_.push_back(std::format("{}", std::move(name)));
}

auto events::operator[](const std::size_t idx) const noexcept -> event {
    return event{ time_points_[idx], names_[idx] };
}

auto events::at(const std::size_t idx) const -> event {
    if (idx >= this->num_events()) {
        throw std::out_of_range{ std::format("Index {} is out-of-bounce for the number of events {}!", idx, this->num_events()) };
    }
    return (*this)[idx];
}

std::ostream &operator<<(std::ostream &out, const events::event &e) {
    return out << std::format("time_point: {}\n"
                              "name: {}",
                              e.time_point.time_since_epoch(),
                              e.name);
}

std::ostream &operator<<(std::ostream &out, const events &e) {
    return out << std::format("time_points: [{}]\n"
                              "names: [{}]",
                              detail::join(detail::time_points_to_epoch(e.get_time_points()), ", "),
                              detail::join(e.get_names(), ", "));
}

}  // namespace hws
