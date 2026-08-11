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

}  // namespace

//*************************************************************************************************************************************//
//                                                          general samples                                                            //
//*************************************************************************************************************************************//

bool cray_pm_counters_general_samples::has_samples() const {
    return this->metadata_.has_value();
}

std::string cray_pm_counters_general_samples::generate_yaml_string() const {
    // if no samples are available, return an empty string
    if (!this->has_samples()) {
        return "";
    }

    std::string str{ "general:\n" };
    const std::string root = detail::pm_counters_root().generic_string();

    if (this->metadata_.has_value()) {
        for (const std::string &key : sorted_keys(this->metadata_.value())) {
            str += fmt::format("  {}:\n"
                               "    unit: \"string\"\n"
                               "    source: \"{}/{}\"\n"
                               "    values: [{}]\n",
                               key, root, key, fmt::join(detail::quote(this->metadata_.value().at(key)), ", "));
        }
    }

    return str;
}

std::ostream &operator<<(std::ostream &out, const cray_pm_counters_general_samples &samples) {
    return out << fmt::format("metadata: {}", detail::map_entry_to_string(samples.get_metadata()));
}

//*************************************************************************************************************************************//
//                                                           power samples                                                             //
//*************************************************************************************************************************************//

bool cray_pm_counters_power_samples::has_samples() const {
    return this->energy_counters_.has_value() || this->power_counters_.has_value();
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
                               "    unit: \"J (cumulative)\"\n"
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
                               "    unit: \"W (instantaneous)\"\n"
                               "    source: \"{}/{}\"\n"
                               "    values: [{}]\n",
                               key, root, key, fmt::join(this->power_counters_.value().at(key), ", "));
            if (const std::vector<std::uint64_t> *timestamps = find_timestamps(this->power_timestamps_us_, key); timestamps != nullptr) {
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
                              "power_timestamps_us: {}",
                              detail::map_entry_to_string(samples.get_energy_counters()),
                              detail::map_entry_to_string(samples.get_energy_timestamps_us()),
                              detail::map_entry_to_string(samples.get_power_counters()),
                              detail::map_entry_to_string(samples.get_power_timestamps_us()));
}

}  // namespace hws
