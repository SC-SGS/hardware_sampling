/**
 * @file
 * @author Alexander Van Craen
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Defines the samples gathered from Cray/HPE's `/sys/cray/pm_counters` sysfs interface.
 * @details The exact set of files exposed by `/sys/cray/pm_counters` is Cray/HPE-generation specific, so instead of
 *          hardcoding specific file names, every regular file found below the directory is picked up dynamically
 *          and classified by its (relative) file name and parsed content into a measured energy counter, a measured
 *          power counter, another power related counter (configured caps, derived energy counters), or general
 *          metadata - see `hws::detail::is_energy_counter_key`/`is_power_counter_key`/`is_power_related_counter`.
 *          Everything except the static metadata (`version`, `raw_scan_hz`) is re-read every sampling tick (see
 *          `cray_pm_counters_general_samples` for why even metadata like `freshness` needs to be). Confirmed against
 *          a real HPE Cray EX255a node (`/sys/cray/pm_counters` version 3): 23 files - `energy`/`accel[0-3]_energy`
 *          (measured energy, J), `power`/`accel[0-3]_power` (measured power, W), other power related counters
 *          (`power_cap`/`accel[0-3]_power_cap`, `capped_energy`, `overshoot_energy`), and general metadata
 *          (`overshoot`, `freshness`, `generation`, `raw_scan_hz`, `startup`, `version`). Version 3 telemetry files
 *          (but not the `*_cap` files) contain `"<value> <unit> <timestamp_us> us"`, e.g.
 *          `"2869283299 J 7810213855886 us"` - see `hws::detail::parse_pm_counter`.
 */

#ifndef HWS_CRAY_PM_COUNTERS_PM_COUNTERS_SAMPLES_HPP_
#define HWS_CRAY_PM_COUNTERS_PM_COUNTERS_SAMPLES_HPP_
#pragma once

#include "hws/utility.hpp"  // HWS_SAMPLE_STRUCT_FIXED_MEMBER

#include "fmt/ostream.h"  // fmt::formatter, fmt::ostream_formatter

#include <cstdint>        // std::uint64_t
#include <iosfwd>         // std::ostream forward declaration
#include <string>         // std::string
#include <unordered_map>  // std::unordered_map
#include <vector>         // std::vector

namespace hws {

//*************************************************************************************************************************************//
//                                                          general samples                                                            //
//*************************************************************************************************************************************//

/**
 * @brief Wrapper class for the non-power/energy related `/sys/cray/pm_counters` metadata, e.g. `version`,
 *        `generation`, `raw_scan_hz`, `overshoot`, or `freshness`.
 * @details Static entries (`version`, `raw_scan_hz`, see `hws::detail::is_static_pm_counter_key`) are read only once.
 *          Everything else is sampled every tick: `freshness` increments at `raw_scan_hz` and `generation`/
 *          `overshoot` change when a power cap is (re)applied or exceeded. HPE's documented consistency check is to
 *          read `freshness` before and after a batch of energy/power reads; if it's unchanged, that batch is a
 *          consistent snapshot - only possible if it's re-sampled. Numeric entries are stored as numbers (with their
 *          unit in `units`), only entries that aren't numeric at all fall back to their raw string content.
 */
class cray_pm_counters_general_samples {
    // befriend hardware sampler class
    friend class cray_pm_counters_hardware_sampler;
    /// The map type used for the static numeric metadata entries: file name (relative to `/sys/cray/pm_counters`) -> value read once.
    using fixed_metadata_type = std::unordered_map<std::string, double>;
    /// The map type used for the sampled numeric metadata entries: file name (relative to `/sys/cray/pm_counters`) -> sampled values.
    using numeric_metadata_type = std::unordered_map<std::string, std::vector<double>>;
    /// The map type used for the sampled non-numeric metadata entries: file name (relative to `/sys/cray/pm_counters`) -> sampled raw file content.
    using string_metadata_type = std::unordered_map<std::string, std::vector<std::string>>;
    /// The map type used for the units of the numeric metadata entries: file name (relative to `/sys/cray/pm_counters`) -> unit.
    using unit_map_type = std::unordered_map<std::string, std::string>;

  public:
    /**
     * @brief Checks whether any general related hardware sample is present.
     * @return `true` if any general related hardware sample is, otherwise `false`.
     */
    [[nodiscard]] bool has_samples() const;
    /**
     * @brief Assemble the YAML string containing all available general hardware samples.
     * @details Returns an empty string if `has_samples()` returns `false`.
     * @return the YAML string (`[[nodiscard]]`)
     */
    [[nodiscard]] std::string generate_yaml_string() const;

    HWS_SAMPLE_STRUCT_FIXED_MEMBER(fixed_metadata_type, fixed_metadata)      // static numeric pm_counters metadata entries, read once (e.g. version, raw_scan_hz)
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(numeric_metadata_type, numeric_metadata)  // sampled numeric pm_counters metadata entries (e.g. freshness, generation, overshoot); NaN if a reading couldn't be parsed
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(string_metadata_type, string_metadata)    // sampled non-numeric pm_counters metadata entries, as raw file content
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(unit_map_type, units)                     // the units of all fixed and numeric metadata entries
};

/**
 * @brief Output the general @p samples to the given output-stream @p out.
 * @param[in,out] out the output-stream to write the general related hardware samples to
 * @param[in] samples the pm_counters general related samples
 * @return the output-stream
 */
std::ostream &operator<<(std::ostream &out, const cray_pm_counters_general_samples &samples);

//*************************************************************************************************************************************//
//                                                           power samples                                                             //
//*************************************************************************************************************************************//

/**
 * @brief Wrapper class for the dynamically discovered energy and power counters below `/sys/cray/pm_counters`
 *        (e.g. `energy`/`power` for the whole node, `accel[i]_energy`/`accel[i]_power` per APU).
 * @details Values are cumulative Joules for energy counters and instantaneous Watts for power counters (confirmed
 *          against a real HPE Cray EX255a node). On PM counters version 3, telemetry files (but not the `*_cap`
 *          files) additionally carry a per-sample microsecond timestamp (see `energy_timestamps_us`/
 *          `power_timestamps_us`), latched by the HSS - use this instead of the host read time for accurate
 *          energy-to-power derivatives, since pm_counters only updates at `raw_scan_hz` (10 Hz).
 *          All other power/energy related numeric entries that aren't measured telemetry (configured caps like
 *          `power_cap`/`accel[i]_power_cap`, derived counters like `capped_energy`/`overshoot_energy`, see
 *          `hws::detail::is_power_related_counter`) are stored in `other_counters` with their unit in `other_units`.
 */
class cray_pm_counters_power_samples {
    // befriend hardware sampler class
    friend class cray_pm_counters_hardware_sampler;
    /// The map type used for the dynamically discovered counters: file name (relative to `/sys/cray/pm_counters`) -> sampled raw values.
    using counter_map_type = std::unordered_map<std::string, std::vector<std::uint64_t>>;
    /// The map type used for the other power related counters: file name (relative to `/sys/cray/pm_counters`) -> sampled values.
    using other_counter_map_type = std::unordered_map<std::string, std::vector<double>>;
    /// The map type used for the units of the other power related counters: file name (relative to `/sys/cray/pm_counters`) -> unit.
    using unit_map_type = std::unordered_map<std::string, std::string>;

  public:
    /**
     * @brief Checks whether any power related hardware sample is present.
     * @return `true` if any power related hardware sample is, otherwise `false`.
     */
    [[nodiscard]] bool has_samples() const;
    /**
     * @brief Assemble the YAML string containing all available power hardware samples.
     * @details Returns an empty string if `has_samples()` returns `false`.
     * @return the YAML string (`[[nodiscard]]`)
     */
    [[nodiscard]] std::string generate_yaml_string() const;

    HWS_SAMPLE_STRUCT_FIXED_MEMBER(counter_map_type, energy_counters)       // dynamically discovered *energy* pm_counters entries, in J
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(counter_map_type, power_counters)        // dynamically discovered *power* pm_counters entries, in W
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(counter_map_type, energy_timestamps_us)  // per-sample HSS latch timestamp for energy_counters, in us (PM counters v3+)
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(counter_map_type, power_timestamps_us)   // per-sample HSS latch timestamp for power_counters, in us (PM counters v3+)
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(other_counter_map_type, other_counters)  // other power related pm_counters entries (e.g. power caps, capped/overshoot energy); NaN if a reading couldn't be parsed
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(counter_map_type, other_timestamps_us)   // per-sample HSS latch timestamp for other_counters, in us (if the file provides one)
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(unit_map_type, other_units)              // the units of the other_counters entries
};

/**
 * @brief Output the power related @p samples to the given output-stream @p out.
 * @param[in,out] out the output-stream to write the power related hardware samples to
 * @param[in] samples the pm_counters power related samples
 * @return the output-stream
 */
std::ostream &operator<<(std::ostream &out, const cray_pm_counters_power_samples &samples);

}  // namespace hws

/// @cond Doxygen_suppress

template <>
struct fmt::formatter<hws::cray_pm_counters_general_samples> : fmt::ostream_formatter { };

template <>
struct fmt::formatter<hws::cray_pm_counters_power_samples> : fmt::ostream_formatter { };

/// @endcond

#endif  // HWS_CRAY_PM_COUNTERS_PM_COUNTERS_SAMPLES_HPP_
