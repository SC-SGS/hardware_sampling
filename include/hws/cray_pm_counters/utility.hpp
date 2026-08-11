/**
 * @file
 * @author Alexander Van Craen
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Utility functions to discover and read Cray/HPE's `/sys/cray/pm_counters` sysfs interface.
 */

#ifndef HWS_CRAY_PM_COUNTERS_UTILITY_HPP_
#define HWS_CRAY_PM_COUNTERS_UTILITY_HPP_
#pragma once

#include <cstdint>      // std::uint64_t
#include <filesystem>   // std::filesystem::path
#include <optional>     // std::optional
#include <string>       // std::string
#include <string_view>  // std::string_view
#include <vector>       // std::vector

namespace hws::detail {

/// The default root directory of Cray/HPE's pm_counters sysfs interface.
inline constexpr std::string_view default_pm_counters_root = "/sys/cray/pm_counters";

/**
 * @brief Return the root directory of the pm_counters sysfs interface to use.
 * @details Defaults to `hws::detail::default_pm_counters_root`, but can be overridden via the `HWS_PM_COUNTERS_PATH`
 *          environment variable (e.g. to point at a synthetic directory for local testing off a Cray system).
 * @return the pm_counters root directory (`[[nodiscard]]`)
 */
[[nodiscard]] std::filesystem::path pm_counters_root();

/**
 * @brief Check whether the pm_counters sysfs interface is available on this node.
 * @return `true` if `hws::detail::pm_counters_root()` exists and is a directory, otherwise `false` (`[[nodiscard]]`)
 */
[[nodiscard]] bool pm_counters_available();

/**
 * @brief Recursively list all regular files below `hws::detail::pm_counters_root()`.
 * @return the discovered files, or an empty vector if pm_counters isn't available (`[[nodiscard]]`)
 */
[[nodiscard]] std::vector<std::filesystem::path> list_pm_counter_files();

/**
 * @brief Derive the unique map key for @p file: its path relative to `hws::detail::pm_counters_root()`, e.g.
 *        `energy` or, for a nested file, `accel0/energy` ('/' is a valid, unquoted YAML mapping key character).
 * @param[in] file the pm_counters file, must be located below `hws::detail::pm_counters_root()`
 * @return the map key (`[[nodiscard]]`)
 */
[[nodiscard]] std::string pm_counter_key(const std::filesystem::path &file);

/**
 * @brief Read the trimmed content of @p file as a raw string.
 * @param[in] file the file to read
 * @return the trimmed file content, or an empty string if the file couldn't be read (`[[nodiscard]]`)
 */
[[nodiscard]] std::string read_pm_counter_raw(const std::filesystem::path &file);

/**
 * @brief A single pm_counters energy/power reading.
 * @details On Hunter (PM counters version 3), telemetry files (e.g. `energy`, `accel0_power`) contain
 *          `"<value> <unit> <timestamp_us> us"` (e.g. `"2869283299 J 7810213855886 us"`), while power/energy cap
 *          files and older PM counter versions may only contain `"<value> <unit>"` or even just `"<value>"`.
 */
struct pm_counter_reading {
    /// The counter's raw value (unit depends on the file, e.g. J for energy, W for power).
    std::uint64_t value{};
    /// The microsecond timestamp at which the value was latched by the HSS, if the file provides one.
    std::optional<std::uint64_t> timestamp_us{};
};

/**
 * @brief Read the content of @p file and try to parse it as a `pm_counter_reading`.
 * @param[in] file the file to read
 * @return the parsed reading, or `std::nullopt` if the file couldn't be read or doesn't start with an unsigned integer (`[[nodiscard]]`)
 */
[[nodiscard]] std::optional<pm_counter_reading> read_pm_counter_reading(const std::filesystem::path &file);

/**
 * @brief Check whether the map key @p key (as returned by `pm_counter_key`) looks like a measured energy counter:
 *        contains "energy" but not "cap" or "overshoot" (e.g. `energy`/`accel0_energy`, but not `capped_energy`
 *        or `overshoot_energy`, which are derived/event counters, not raw measured node/accelerator energy).
 * @param[in] key the map key to check
 * @return `true` if @p key looks like a measured energy counter, otherwise `false` (`[[nodiscard]]`)
 */
[[nodiscard]] bool is_energy_counter_key(const std::string &key);

/**
 * @brief Check whether the map key @p key (as returned by `pm_counter_key`) looks like a measured power counter:
 *        contains "power" but not "cap" or "overshoot" (e.g. `power`/`accel0_power`, but not `power_cap`/
 *        `accel0_power_cap`, which are configured limits, not measured power draw).
 * @param[in] key the map key to check
 * @return `true` if @p key looks like a measured power counter, otherwise `false` (`[[nodiscard]]`)
 */
[[nodiscard]] bool is_power_counter_key(const std::string &key);

}  // namespace hws::detail

#endif  // HWS_CRAY_PM_COUNTERS_UTILITY_HPP_
