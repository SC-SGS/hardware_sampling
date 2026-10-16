/**
 * @author Alexander Van Craen
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hws/cray_pm_counters/pm_counters_samples.hpp"

#include "hws/cray_pm_counters/utility.hpp"  // hws::detail::pm_counters_root
#include "hws/utility.hpp"                   // hws::detail::map_entry_to_string

#include "fmt/format.h"  // fmt::format
#include "fmt/ranges.h"  // fmt::join

#include <algorithm>      // std::sort
#include <cmath>          // std::isnan
#include <cstdint>        // std::uint64_t
#include <optional>       // std::optional
#include <ostream>        // std::ostream
#include <string>         // std::string
#include <unordered_map>  // std::unordered_map
#include <vector>         // std::vector

namespace hws {

namespace {

// unordered_map has no defined iteration order; sort keys so the generated YAML is reproducible across runs.
template <typename MapType>
[[nodiscard]] std::vector<std::string> sorted_keys(const MapType &map) {
    std::vector<std::string> keys{};
    keys.reserve(map.size());
    for (const auto &[key, value] : map) {
        keys.push_back(key);
    }
    std::sort(keys.begin(), keys.end());
    return keys;
}

// format a single number for the YAML output; NaN (an unparsable reading) is written as YAML's ".nan"
[[nodiscard]] std::string format_number(const double value) {
    return std::isnan(value) ? std::string{ ".nan" } : fmt::format("{}", value);
}

// format all numbers for the YAML output, see format_number
[[nodiscard]] std::string format_numbers(const std::vector<double> &values) {
    std::vector<std::string> formatted{};
    formatted.reserve(values.size());
    for (const double value : values) {
        formatted.push_back(format_number(value));
    }
    return fmt::format("{}", fmt::join(formatted, ", "));
}

// look up the unit of the entry key; "int" if none is known (shouldn't happen, every numeric entry gets a unit)
template <typename UnitMapType>
[[nodiscard]] std::string unit_of(const std::optional<UnitMapType> &units, const std::string &key) {
    if (units.has_value()) {
        if (const auto it = units.value().find(key); it != units.value().cend()) {
            return it->second;
        }
    }
    return "int";
}

}  // namespace

//*************************************************************************************************************************************//
//                                                          general samples                                                            //
//*************************************************************************************************************************************//

bool cray_pm_counters_general_samples::has_samples() const {
    return this->fixed_metadata_.has_value() || this->numeric_metadata_.has_value() || this->string_metadata_.has_value();
}

std::string cray_pm_counters_general_samples::generate_yaml_string() const {
    // if no samples are available, return an empty string
    if (!this->has_samples()) {
        return "";
    }

    std::string str{ "general:\n" };
    const std::string root = detail::pm_counters_root().generic_string();

    // emit all entries sorted by name, regardless of whether they are fixed, numeric, or string entries
    std::vector<std::string> keys{};
    if (this->fixed_metadata_.has_value()) {
        const std::vector<std::string> fixed_keys = sorted_keys(this->fixed_metadata_.value());
        keys.insert(keys.end(), fixed_keys.cbegin(), fixed_keys.cend());
    }
    if (this->numeric_metadata_.has_value()) {
        const std::vector<std::string> numeric_keys = sorted_keys(this->numeric_metadata_.value());
        keys.insert(keys.end(), numeric_keys.cbegin(), numeric_keys.cend());
    }
    if (this->string_metadata_.has_value()) {
        const std::vector<std::string> string_keys = sorted_keys(this->string_metadata_.value());
        keys.insert(keys.end(), string_keys.cbegin(), string_keys.cend());
    }
    std::sort(keys.begin(), keys.end());

    for (const std::string &key : keys) {
        if (this->fixed_metadata_.has_value() && this->fixed_metadata_.value().count(key) != 0) {
            str += fmt::format("  {}:\n"
                               "    unit: \"{}\"\n"
                               "    source: \"{}/{}\"\n"
                               "    values: {}\n",
                               key, unit_of(this->units_, key), root, key, format_number(this->fixed_metadata_.value().at(key)));
        } else if (this->numeric_metadata_.has_value() && this->numeric_metadata_.value().count(key) != 0) {
            str += fmt::format("  {}:\n"
                               "    unit: \"{}\"\n"
                               "    source: \"{}/{}\"\n"
                               "    values: [{}]\n",
                               key, unit_of(this->units_, key), root, key, format_numbers(this->numeric_metadata_.value().at(key)));
        } else {
            str += fmt::format("  {}:\n"
                               "    unit: \"string\"\n"
                               "    source: \"{}/{}\"\n"
                               "    values: [{}]\n",
                               key, root, key, fmt::join(detail::quote(this->string_metadata_.value().at(key)), ", "));
        }
    }

    return str;
}

std::ostream &operator<<(std::ostream &out, const cray_pm_counters_general_samples &samples) {
    return out << fmt::format("fixed_metadata: {}\n"
                              "numeric_metadata: {}\n"
                              "string_metadata: {}\n"
                              "units: {}",
                              detail::map_entry_to_string(samples.get_fixed_metadata()),
                              detail::map_entry_to_string(samples.get_numeric_metadata()),
                              detail::map_entry_to_string(samples.get_string_metadata()),
                              detail::map_entry_to_string(samples.get_units()));
}

//*************************************************************************************************************************************//
//                                                           power samples                                                             //
//*************************************************************************************************************************************//

bool cray_pm_counters_power_samples::has_samples() const {
    return this->energy_counters_.has_value() || this->power_counters_.has_value() || this->other_counters_.has_value();
}

std::string cray_pm_counters_power_samples::generate_yaml_string() const {
    // if no samples are available, return an empty string
    if (!this->has_samples()) {
        return "";
    }

    std::string str{ "power:\n" };
    const std::string root = detail::pm_counters_root().generic_string();

    // look up the per-sample timestamps (if any) recorded for the counter map key; nullptr if none are available
    const auto find_timestamps = [](const std::optional<counter_map_type> &timestamps, const std::string &key) -> const std::vector<std::uint64_t> * {
        if (!timestamps.has_value()) {
            return nullptr;
        }
        const auto it = timestamps.value().find(key);
        return it != timestamps.value().cend() ? &it->second : nullptr;
    };

    if (this->energy_counters_.has_value()) {
        for (const std::string &key : sorted_keys(this->energy_counters_.value())) {
            str += fmt::format("  {}:\n"
                               "    unit: \"J\"\n"
                               "    source: \"{}/{}\"\n"
                               "    values: [{}]\n",
                               key, root, key, fmt::join(this->energy_counters_.value().at(key), ", "));
            if (const std::vector<std::uint64_t> *timestamps = find_timestamps(this->energy_timestamps_us_, key); timestamps != nullptr) {
                str += fmt::format("    timestamps_us: [{}]\n", fmt::join(*timestamps, ", "));
            }
        }
    }
    if (this->power_counters_.has_value()) {
        for (const std::string &key : sorted_keys(this->power_counters_.value())) {
            str += fmt::format("  {}:\n"
                               "    unit: \"W\"\n"
                               "    source: \"{}/{}\"\n"
                               "    values: [{}]\n",
                               key, root, key, fmt::join(this->power_counters_.value().at(key), ", "));
            if (const std::vector<std::uint64_t> *timestamps = find_timestamps(this->power_timestamps_us_, key); timestamps != nullptr) {
                str += fmt::format("    timestamps_us: [{}]\n", fmt::join(*timestamps, ", "));
            }
        }
    }
    if (this->other_counters_.has_value()) {
        for (const std::string &key : sorted_keys(this->other_counters_.value())) {
            str += fmt::format("  {}:\n"
                               "    unit: \"{}\"\n"
                               "    source: \"{}/{}\"\n"
                               "    values: [{}]\n",
                               key, unit_of(this->other_units_, key), root, key, format_numbers(this->other_counters_.value().at(key)));
            if (const std::vector<std::uint64_t> *timestamps = find_timestamps(this->other_timestamps_us_, key); timestamps != nullptr && !timestamps->empty()) {
                str += fmt::format("    timestamps_us: [{}]\n", fmt::join(*timestamps, ", "));
            }
        }
    }

    return str;
}

std::ostream &operator<<(std::ostream &out, const cray_pm_counters_power_samples &samples) {
    return out << fmt::format("energy_counters: {}\n"
                              "energy_timestamps_us: {}\n"
                              "power_counters: {}\n"
                              "power_timestamps_us: {}\n"
                              "other_counters: {}\n"
                              "other_timestamps_us: {}\n"
                              "other_units: {}",
                              detail::map_entry_to_string(samples.get_energy_counters()),
                              detail::map_entry_to_string(samples.get_energy_timestamps_us()),
                              detail::map_entry_to_string(samples.get_power_counters()),
                              detail::map_entry_to_string(samples.get_power_timestamps_us()),
                              detail::map_entry_to_string(samples.get_other_counters()),
                              detail::map_entry_to_string(samples.get_other_timestamps_us()),
                              detail::map_entry_to_string(samples.get_other_units()));
}

}  // namespace hws
