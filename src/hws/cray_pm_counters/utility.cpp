/**
 * @author Alexander Van Craen
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hws/cray_pm_counters/utility.hpp"

#include "hws/utility.hpp"  // hws::detail::{trim, to_lower_case, split, is_integer, convert_to}

#include "fmt/format.h"  // fmt::format
#include "fmt/ranges.h"  // fmt::join

#include <algorithm>    // std::sort, std::unique
#include <cstddef>      // std::size_t
#include <cstdint>      // std::uint64_t
#include <cstdlib>      // std::getenv
#include <filesystem>   // std::filesystem::{path, directory_entry, exists, is_directory, recursive_directory_iterator, relative}
#include <fstream>      // std::ifstream
#include <optional>     // std::optional, std::nullopt
#include <sstream>      // std::ostringstream
#include <string>       // std::string
#include <string_view>  // std::string_view
#include <system_error> // std::error_code
#include <utility>      // std::pair
#include <vector>       // std::vector

namespace hws::detail {

std::filesystem::path pm_counters_root() {
    // allow overriding the pm_counters root for local testing (e.g. against a synthetic directory off a Cray system)
    if (const char *override_path = std::getenv("HWS_PM_COUNTERS_PATH"); override_path != nullptr) {
        return std::filesystem::path{ override_path };
    }
    return std::filesystem::path{ default_pm_counters_root };
}

bool pm_counters_available() {
    const std::filesystem::path root = pm_counters_root();
    std::error_code ec{};
    return std::filesystem::exists(root, ec) && std::filesystem::is_directory(root, ec);
}

std::vector<std::filesystem::path> list_pm_counter_files() {
    std::vector<std::filesystem::path> files{};

    if (!pm_counters_available()) {
        return files;
    }

    std::error_code ec{};
    for (const std::filesystem::directory_entry &entry : std::filesystem::recursive_directory_iterator(pm_counters_root(), std::filesystem::directory_options::skip_permission_denied, ec)) {
        if (entry.is_regular_file(ec)) {
            files.push_back(entry.path());
        }
    }

    return files;
}

std::string pm_counter_key(const std::filesystem::path &file) {
    std::error_code ec{};
    std::filesystem::path relative = std::filesystem::relative(file, pm_counters_root(), ec);
    if (ec) {
        relative = file.filename();
    }

    // '/' has no special meaning in YAML mapping keys, so a nested file's relative path (e.g. "accel0/energy") is
    // used as-is; this also means the key always doubles as the real path relative to pm_counters_root() for the
    // "source:" field in the generated YAML. All confirmed real HPE pm_counters layouts (2014 CUG slides, 2024 CUG
    // slides, a real Hunter node) are flat (no subdirectories), so this rarely matters in practice.
    return relative.generic_string();
}

std::string read_pm_counter_raw(const std::filesystem::path &file) {
    std::ifstream stream{ file };
    if (!stream.is_open()) {
        return "";
    }

    std::ostringstream content{};
    content << stream.rdbuf();
    return std::string{ detail::trim(content.str()) };
}

std::optional<pm_counter_reading> read_pm_counter_reading(const std::filesystem::path &file) {
    const std::string content = read_pm_counter_raw(file);
    if (content.empty()) {
        return std::nullopt;
    }

    const std::vector<std::string_view> tokens = detail::split(content, ' ');
    if (tokens.empty()) {
        return std::nullopt;
    }

    pm_counter_reading reading{};
    try {
        reading.value = detail::convert_to<std::uint64_t>(tokens.front());
    } catch (const std::exception &) {
        return std::nullopt;
    }

    // PM counters version 3 appends a microsecond timestamp as "... <timestamp_us> us" to telemetry (but not cap) files
    if (tokens.size() >= 2 && tokens.back() == "us" && detail::is_integer(tokens[tokens.size() - 2])) {
        try {
            reading.timestamp_us = detail::convert_to<std::uint64_t>(tokens[tokens.size() - 2]);
        } catch (const std::exception &) {
            // ignore an unparsable timestamp, the value itself was already parsed successfully
        }
    }

    return reading;
}

namespace {

// Check whether lower_case_key's last path component (i.e. after the last '_' or '/', or the whole key if it has
// neither) is exactly component - e.g. "accel0_energy" and "energy" both match component "energy", but
// "power_state" or "energy_source" don't match component "power"/"energy" respectively. Anchoring on the last
// component (rather than a plain substring search) avoids misclassifying a future metadata field that merely
// contains "energy"/"power" as a substring without actually being a measured energy/power counter.
[[nodiscard]] bool last_component_is(const std::string &lower_case_key, const std::string &component) {
    if (lower_case_key == component) {
        return true;
    }
    const std::size_t suffix_size = component.size() + 1;  // +1 for the separator
    if (lower_case_key.size() <= suffix_size) {
        return false;
    }
    const char separator = lower_case_key[lower_case_key.size() - suffix_size];
    return (separator == '_' || separator == '/') && lower_case_key.compare(lower_case_key.size() - component.size(), component.size(), component) == 0;
}

// "power_cap"/"accel[i]_power_cap" are configured limits (not measured draw), and "capped_energy"/"overshoot"/
// "overshoot_energy" are derived/event counters (confirmed on a real Hunter node) - none of them belong in the
// same map as the actual measured energy/power telemetry this backend exists to compare against hws's own
// software-side measurements, so they're excluded here and fall through to general metadata instead.
[[nodiscard]] bool is_excluded_from_telemetry(const std::string &lower_case_key) {
    return lower_case_key.find("cap") != std::string::npos || lower_case_key.find("overshoot") != std::string::npos;
}

}  // namespace

bool is_energy_counter_key(const std::string &key) {
    const std::string lower_case_key = detail::to_lower_case(key);
    return last_component_is(lower_case_key, "energy") && !is_excluded_from_telemetry(lower_case_key);
}

bool is_power_counter_key(const std::string &key) {
    const std::string lower_case_key = detail::to_lower_case(key);
    return last_component_is(lower_case_key, "power") && !is_excluded_from_telemetry(lower_case_key);
}

std::optional<int> accel_index_from_counter_key(const std::string &key) {
    const std::string lower_case_key = detail::to_lower_case(key);
    constexpr std::string_view prefix = "accel";
    constexpr std::string_view suffixes[] = { "_energy", "_power" };

    if (lower_case_key.compare(0, prefix.size(), prefix) != 0) {
        return std::nullopt;
    }

    for (const std::string_view suffix : suffixes) {
        if (lower_case_key.size() <= prefix.size() + suffix.size()) {
            continue;
        }
        if (lower_case_key.compare(lower_case_key.size() - suffix.size(), suffix.size(), suffix) != 0) {
            continue;
        }
        const std::string digits = lower_case_key.substr(prefix.size(), lower_case_key.size() - prefix.size() - suffix.size());
        if (!digits.empty() && detail::is_integer(digits)) {
            try {
                return detail::convert_to<int>(digits);
            } catch (const std::exception &) {
                return std::nullopt;
            }
        }
    }
    return std::nullopt;
}

std::vector<int> accel_indices_from_counter_keys(const std::vector<std::string> &counter_keys) {
    std::vector<int> indices{};
    for (const std::string &key : counter_keys) {
        if (const std::optional<int> idx = accel_index_from_counter_key(key); idx.has_value()) {
            indices.push_back(*idx);
        }
    }
    std::sort(indices.begin(), indices.end());
    indices.erase(std::unique(indices.begin(), indices.end()), indices.end());
    return indices;
}

std::optional<int> guess_accel_index(const std::string &pci_bus_id,
                                      const std::vector<std::string> &physical_pci_bus_ids_sorted,
                                      const std::vector<int> &accel_indices_sorted) {
    if (physical_pci_bus_ids_sorted.empty() || physical_pci_bus_ids_sorted.size() != accel_indices_sorted.size()) {
        return std::nullopt;
    }
    const auto it = std::find(physical_pci_bus_ids_sorted.cbegin(), physical_pci_bus_ids_sorted.cend(), pci_bus_id);
    if (it == physical_pci_bus_ids_sorted.cend()) {
        return std::nullopt;
    }
    const auto position = static_cast<std::size_t>(std::distance(physical_pci_bus_ids_sorted.cbegin(), it));
    return accel_indices_sorted[position];
}

std::string accel_correlation_yaml_block(const std::string &vendor,
                                          const std::vector<std::pair<std::size_t, std::string>> &visible_devices,
                                          const std::vector<std::string> &physical_pci_bus_ids_sorted,
                                          const std::vector<int> &accel_indices_sorted) {
    const bool topology_matches = !physical_pci_bus_ids_sorted.empty() && physical_pci_bus_ids_sorted.size() == accel_indices_sorted.size();

    std::vector<std::string> visible_entries{};
    for (const auto &[local_index, pci_bus_id] : visible_devices) {
        const std::optional<int> guessed = guess_accel_index(pci_bus_id, physical_pci_bus_ids_sorted, accel_indices_sorted);
        const std::string guessed_str = guessed.has_value() ? std::to_string(*guessed) : "null";
        visible_entries.push_back(fmt::format("        - local_index: {}\n"
                                              "          pci_bus_id: \"{}\"\n"
                                              "          guessed_accel_index: {}",
                                              local_index, pci_bus_id, guessed_str));
    }

    std::vector<std::string> quoted_physical_ids{};
    for (const std::string &id : physical_pci_bus_ids_sorted) {
        quoted_physical_ids.push_back(fmt::format("\"{}\"", id));
    }

    return fmt::format("    {}:\n"
                       "      topology_count_mismatch: {}\n"
                       "      physical_pci_bus_ids: [{}]\n"
                       "      visible_gpus:\n"
                       "{}\n",
                       vendor, !topology_matches, fmt::join(quoted_physical_ids, ", "), fmt::join(visible_entries, "\n"));
}

}  // namespace hws::detail
