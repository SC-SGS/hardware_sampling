/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hardware_sampling/hardware_sampler.hpp"

#include "hardware_sampling/event.hpp"    // hws::event
#include "hardware_sampling/utility.hpp"  // hws::detail::durations_from_reference_time
#include "hardware_sampling/version.hpp"  // hws::version::version

#include "fmt/chrono.h"  // direct formatting of std::chrono types
#include "fmt/format.h"  // fmt::format
#include "fmt/ranges.h"  // fmt::join

#include <chrono>     // std::chrono::{system_clock, steady_clock, duration_cast, milliseconds}
#include <cstddef>    // std::size_t
#include <exception>  // std::exception
#include <fstream>    // std::ofstream
#include <iostream>   // std::cerr, std::endl
#include <stdexcept>  // std::runtime_error, std::out_of_range
#include <thread>     // std::thread
#include <utility>    // std::move

namespace hws {

hardware_sampler::hardware_sampler(const std::chrono::milliseconds sampling_interval, const sample_category category) :
    sampling_interval_{ sampling_interval },
    sample_category_{ category } { }

hardware_sampler::~hardware_sampler() = default;

void hardware_sampler::start_sampling() {
    // can't start an already running sampler
    if (this->has_sampling_started()) {
        throw std::runtime_error{ "Can start every hardware sampler only once!" };
    }

    // record start time
    start_date_time_ = std::chrono::system_clock::now();

    // start sampling loop
    sampling_started_ = true;
    sampling_running_ = true;
    this->add_event("sampling_started");
    sampling_thread_ = std::thread{
        [this]() {
            try {
                this->sampling_loop();
            } catch (const std::exception &e) {
                // print useful error message
                std::cerr << e.what() << std::endl;
                throw;
            }
        }
    };
}

void hardware_sampler::stop_sampling() {
    // can't stop a hardware sampler that has never been started
    if (!this->has_sampling_started()) {
        throw std::runtime_error{ "Can't stop a hardware sampler that has never been started!" };
    }
    // can't stop an already stopped sampler
    if (this->has_sampling_stopped()) {
        throw std::runtime_error{ "Can stop every hardware sampler only once!" };
    }

    // stop sampling
    sampling_running_ = false;
    sampling_stopped_ = true;  // -> notifies the sampling std::thread
    sampling_thread_.join();
    this->add_event("sampling_stopped");
}

void hardware_sampler::pause_sampling() {
    sampling_running_ = false;  // notifies the sampling std::thread
    this->add_event("sampling_paused");
}

void hardware_sampler::resume_sampling() {
    if (this->has_sampling_stopped()) {
        throw std::runtime_error{ "Can't resume a hardware sampler that has already been stopped!" };
    }
    sampling_running_ = true;  // notifies the sampling std::thread
    this->add_event("sampling_resumed");
}

bool hardware_sampler::has_sampling_started() const noexcept {
    return sampling_started_;
}

bool hardware_sampler::is_sampling() const noexcept {
    return sampling_running_;
}

bool hardware_sampler::has_sampling_stopped() const noexcept {
    return sampling_stopped_;
}

void hardware_sampler::add_event(event e) {
    events_.push_back(std::move(e));
}

void hardware_sampler::add_event(decltype(event::time_point) time_point, decltype(event::name) name) {
    events_.emplace_back(time_point, name);
}

void hardware_sampler::add_event(decltype(event::name) name) {
    events_.emplace_back(std::chrono::steady_clock::now(), name);
}

event hardware_sampler::get_event(const std::size_t idx) const {
    if (idx >= this->num_events()) {
        throw std::out_of_range{ fmt::format("The index {} is out-of-range for the number of events {}!", idx, this->num_events()) };
    }

    return events_[idx];
}

void hardware_sampler::dump_yaml(const char *filename) const {
    if (!this->has_sampling_stopped()) {
        throw std::runtime_error{ "Can dump samples to the YAML file only after the sampling has been stopped!" };
    }

    std::ofstream file{ filename, std::ios_base::app };

    // begin a new YAML document (only with "---" multiple YAML documents in a single file are allowed)
    file << "---\n\n"
         << this->as_yaml_string();
}

void hardware_sampler::dump_yaml(const std::string &filename) const {
    this->dump_yaml(filename.c_str());
}

void hardware_sampler::dump_yaml(const std::filesystem::path &filename) const {
    this->dump_yaml(filename.string().c_str());
}

std::string hardware_sampler::as_yaml_string() const {
    if (!this->has_sampling_stopped()) {
        throw std::runtime_error{ "Can return samples as string only after the sampling has been stopped!" };
    }

    // generate the event information
    std::vector<decltype(event::time_point)> event_time_points{};
    std::vector<decltype(event::name)> event_names{};
    for (const auto &[time_point, name] : events_) {
        event_time_points.push_back(time_point);
        event_names.push_back(fmt::format("\"{}\"", name));
    }

    return fmt::format("device_identification: \"{}\"\n"
                       "\n"
                       "version: \"{}\"\n"
                       "\n"
                       "start_time: \"{:%Y-%m-%d %X}\"\n"
                       "\n"
                       "events:\n"
                       "  time_points:\n"
                       "    unit: \"s\"\n"
                       "    values: [{}]\n"
                       "  names: [{}]\n"
                       "\n"
                       "sampling_interval:\n"
                       "  unit: \"ms\"\n"
                       "  values: {}\n"
                       "\n"
                       "time_points:\n"
                       "  unit: \"s\"\n"
                       "  values: [{}]\n"
                       "\n"
                       "{}\n",
                       this->device_identification(),
                       version::version,
                       start_date_time_,
                       fmt::join(detail::durations_from_reference_time(event_time_points, this->get_event(0).time_point), ", "),
                       fmt::join(event_names, ", "),
                       this->sampling_interval().count(),
                       fmt::join(detail::durations_from_reference_time(this->sampling_time_points(), this->get_event(0).time_point), ", "),
                       this->samples_only_as_yaml_string());
}

void hardware_sampler::add_time_point(const std::chrono::steady_clock::time_point time_point) {
    time_points_.push_back(time_point);
}

bool hardware_sampler::sample_category_enabled(const sample_category category) const noexcept {
    return static_cast<int>(this->sample_category_ & category) != 0;
}

}  // namespace hws
