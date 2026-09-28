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
 *          and classified by its (relative) file name into a measured energy counter, a measured power counter, or
 *          (everything else, including configured caps and derived/event counters) general metadata - see
 *          `hws::detail::is_energy_counter_key`/`is_power_counter_key`. All three categories are re-read every
 *          sampling tick (see `general_samples()` for why even seemingly static metadata like `freshness` needs to
 *          be). Confirmed against a real HPE Cray EX255a node (`/sys/cray/pm_counters` version 3): 23 files -
 *          `energy`/`accel[0-3]_energy` (measured energy, J), `power`/`accel[0-3]_power` (measured power, W), and
 *          general metadata covering both configured limits (`power_cap`/`accel[0-3]_power_cap`), derived/event
 *          counters (`capped_energy`, `overshoot`, `overshoot_energy`), and protocol metadata (`freshness`,
 *          `generation`, `raw_scan_hz`, `startup`, `version`). Version 3 telemetry files (but not the `*_cap`
 *          files) contain `"<value> <unit> <timestamp_us> us"`, e.g. `"2869283299 J 7810213855886 us"` - see
 *          `pm_counter_reading`.
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
 * @brief Wrapper class for the non-energy, non-power `/sys/cray/pm_counters` metadata, e.g. `version`,
 *        `generation`, `raw_scan_hz`, or `freshness`.
 * @details Sampled every tick, not just once: while `version`/`startup`/`raw_scan_hz` are static, `freshness`
 *          increments at `raw_scan_hz` and `generation`/`overshoot` change when a power cap is (re)applied or
 *          exceeded. HPE's documented consistency check is to read `freshness` before and after a batch of energy
 *          /power reads; if it's unchanged, that batch is a consistent snapshot - only possible if it's re-sampled.
 */
class cray_pm_counters_general_samples {
    // befriend hardware sampler class
    friend class cray_pm_counters_hardware_sampler;
    /// The map type used for the dynamically discovered metadata entries: file name (relative to `/sys/cray/pm_counters`) -> sampled raw file content.
    using metadata_type = std::unordered_map<std::string, std::vector<std::string>>;

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

    HWS_SAMPLE_STRUCT_FIXED_MEMBER(metadata_type, metadata)  // dynamically discovered pm_counters metadata entries
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
 */
class cray_pm_counters_power_samples {
    // befriend hardware sampler class
    friend class cray_pm_counters_hardware_sampler;
    /// The map type used for the dynamically discovered counters: file name (relative to `/sys/cray/pm_counters`) -> sampled raw values.
    using counter_map_type = std::unordered_map<std::string, std::vector<std::uint64_t>>;

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
