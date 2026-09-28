/**
 * @author Alexander Van Craen
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hws/cray_pm_counters/hardware_sampler.hpp"

#include "hws/cray_pm_counters/pm_counters_samples.hpp"  // hws::{cray_pm_counters_general_samples, cray_pm_counters_power_samples}
#include "hws/cray_pm_counters/utility.hpp"               // hws::detail::{pm_counters_available, list_pm_counter_files, pm_counter_key, read_pm_counter_raw, read_pm_counter_reading, pm_counter_reading, is_energy_counter_key, is_power_counter_key, accel_indices_from_counter_keys}
#include "hws/hardware_sampler.hpp"                       // hws::hardware_sampler
#include "hws/sample_category.hpp"                        // hws::sample_category
#include "hws/utility.hpp"                                // hws::detail::time_points_to_epoch

#include "fmt/chrono.h"  // direct formatting of std::chrono types
#include "fmt/format.h"  // fmt::format
#include "fmt/ranges.h"  // fmt::join

#include <algorithm>   // std::max
#include <chrono>      // std::chrono::{steady_clock, milliseconds, duration_cast}
#include <exception>   // std::exception, std::terminate
#include <filesystem>  // std::filesystem::path
#include <ios>         // std::ios_base
#include <iostream>    // std::cerr, std::endl
#include <optional>    // std::optional
#include <ostream>     // std::ostream
#include <stdexcept>   // std::runtime_error
#include <string>      // std::string
#include <thread>      // std::this_thread
#include <vector>      // std::vector

namespace hws {

cray_pm_counters_hardware_sampler::cray_pm_counters_hardware_sampler(const sample_category category) :
    cray_pm_counters_hardware_sampler{ HWS_SAMPLING_INTERVAL, category } { }

cray_pm_counters_hardware_sampler::cray_pm_counters_hardware_sampler(const std::chrono::milliseconds sampling_interval, const sample_category category) :
    hardware_sampler{ sampling_interval, category } { }

cray_pm_counters_hardware_sampler::~cray_pm_counters_hardware_sampler() {
    try {
        // if this hardware sampler is still sampling, stop it
        if (this->has_sampling_started() && !this->has_sampling_stopped()) {
            this->stop_sampling();
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::terminate();
    }
}

void cray_pm_counters_hardware_sampler::sample_once() {
    if (this->sample_category_enabled(sample_category::general)) {
        if (!general_samples_.metadata_.has_value() && !general_paths_.empty()) {
            general_samples_.metadata_ = cray_pm_counters_general_samples::metadata_type{};
        }
        for (const auto &[key, path] : general_paths_) {
            general_samples_.metadata_.value()[key].push_back(detail::read_pm_counter_raw(path));
        }
    }

    if (this->sample_category_enabled(sample_category::power)) {
        if (!power_samples_.energy_counters_.has_value() && !energy_counter_paths_.empty()) {
            power_samples_.energy_counters_ = cray_pm_counters_power_samples::counter_map_type{};
            power_samples_.energy_timestamps_us_ = cray_pm_counters_power_samples::counter_map_type{};
        }
        if (!power_samples_.power_counters_.has_value() && !power_counter_paths_.empty()) {
            power_samples_.power_counters_ = cray_pm_counters_power_samples::counter_map_type{};
            power_samples_.power_timestamps_us_ = cray_pm_counters_power_samples::counter_map_type{};
        }
        for (const auto &[key, path] : energy_counter_paths_) {
            const std::optional<detail::pm_counter_reading> reading = detail::read_pm_counter_reading(path);
            if (reading.has_value()) {
                power_samples_.energy_counters_.value()[key].push_back(reading->value);
                if (reading->timestamp_us.has_value()) {
                    power_samples_.energy_timestamps_us_.value()[key].push_back(reading->timestamp_us.value());
                }
            }
        }
        for (const auto &[key, path] : power_counter_paths_) {
            const std::optional<detail::pm_counter_reading> reading = detail::read_pm_counter_reading(path);
            if (reading.has_value()) {
                power_samples_.power_counters_.value()[key].push_back(reading->value);
                if (reading->timestamp_us.has_value()) {
                    power_samples_.power_timestamps_us_.value()[key].push_back(reading->timestamp_us.value());
                }
            }
        }
    }
}

std::optional<std::chrono::milliseconds> cray_pm_counters_hardware_sampler::hardware_tick_period() const {
    // read directly from the discovered path rather than general_samples_.metadata_: the latter is only populated
    // when sample_category::general is enabled, but throttling to the hardware's real rate must apply regardless
    // of which sample categories the caller requested (e.g. sample_category::power alone).
    const auto it = general_paths_.find("raw_scan_hz");
    if (it == general_paths_.cend()) {
        return std::nullopt;
    }

    const std::string content = detail::read_pm_counter_raw(it->second);
    if (content.empty()) {
        return std::nullopt;
    }

    try {
        const double hz = detail::convert_to<double>(content);
        if (hz <= 0.0) {
            return std::nullopt;
        }
        return std::chrono::milliseconds{ static_cast<std::chrono::milliseconds::rep>(1000.0 / hz) };
    } catch (const std::exception &) {
        return std::nullopt;
    }
}

void cray_pm_counters_hardware_sampler::sampling_loop() {
    //
    // discover the available pm_counters files once and classify them by name into energy counters, power counters,
    // and general metadata; the exact set of files is Cray-generation specific, so every file below
    // /sys/cray/pm_counters is picked up dynamically instead of relying on hardcoded names. All three categories
    // are re-read every tick (including metadata like "freshness", which HPE's own consistency check relies on
    // changing - see cray_pm_counters_general_samples).
    //

    this->add_time_point(std::chrono::steady_clock::now());

    for (const std::filesystem::path &file : detail::list_pm_counter_files()) {
        const std::string key = detail::pm_counter_key(file);

        if (detail::is_energy_counter_key(key)) {
            energy_counter_paths_.emplace(key, file);
        } else if (detail::is_power_counter_key(key)) {
            power_counter_paths_.emplace(key, file);
        } else {
            general_paths_.emplace(key, file);
        }
    }

    this->sample_once();

    // pm_counters itself only updates at raw_scan_hz (10 Hz on Hunter); polling faster than that only yields
    // duplicate values (and needlessly hammers sysfs), so never poll faster than the hardware's own rate - even if
    // the globally configured sampling interval is shorter. Read dynamically rather than hardcoding 10 Hz, since
    // raw_scan_hz can differ across Cray/HPE generations.
    const std::optional<std::chrono::milliseconds> hw_tick_period = this->hardware_tick_period();
    const std::chrono::milliseconds effective_interval = hw_tick_period.has_value() ? std::max(this->sampling_interval(), hw_tick_period.value()) : this->sampling_interval();
    if (hw_tick_period.has_value() && effective_interval > this->sampling_interval()) {
        this->add_event(fmt::format("cray_pm_counters: throttled sampling interval from {} to {} to match the hardware's raw_scan_hz", this->sampling_interval(), effective_interval));
    }

    //
    // loop until stop_sampling() is called
    //

    while (!this->has_sampling_stopped()) {
        // only sample values if the sampler currently isn't paused
        if (this->is_sampling()) {
            // add current time point
            this->add_time_point(std::chrono::steady_clock::now());

            this->sample_once();
        }

        // wait for the (possibly hardware-throttled) sampling interval to pass to retrieve the next sample
        std::this_thread::sleep_for(effective_interval);
    }
}

std::string cray_pm_counters_hardware_sampler::device_identification() const {
    return "cray_pm_counters_device";
}

std::vector<int> cray_pm_counters_hardware_sampler::discovered_accel_indices() const {
    if (!power_samples_.get_energy_counters().has_value()) {
        return {};
    }
    std::vector<std::string> keys{};
    for (const auto &entry : power_samples_.get_energy_counters().value()) {
        keys.push_back(entry.first);
    }
    return detail::accel_indices_from_counter_keys(keys);
}

std::string cray_pm_counters_hardware_sampler::samples_only_as_yaml_string() const {
    // check whether it's safe to generate the YAML entry
    if (this->is_sampling()) {
        throw std::runtime_error{ "Can't create the final YAML entry if the hardware sampler is still running!" };
    }

    return fmt::format("{}{}"
                       "{}",
                       general_samples_.generate_yaml_string(),
                       general_samples_.has_samples() ? "\n" : "",
                       power_samples_.generate_yaml_string());
}

std::ostream &operator<<(std::ostream &out, const cray_pm_counters_hardware_sampler &sampler) {
    if (sampler.is_sampling()) {
        out.setstate(std::ios_base::failbit);
        return out;
    } else {
        return out << fmt::format("sampling interval: {}\n"
                                  "time points: [{}]\n\n"
                                  "general samples:\n{}\n\n"
                                  "power samples:\n{}",
                                  sampler.sampling_interval(),
                                  fmt::join(detail::time_points_to_epoch(sampler.sampling_time_points()), ", "),
                                  sampler.general_samples(),
                                  sampler.power_samples());
    }
}

}  // namespace hws
