/**
 * @file
 * @author Alexander Van Craen
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Defines a hardware sampler for whole-node power/energy using Cray/HPE's `/sys/cray/pm_counters` sysfs
 *        interface (measured, not modeled, node power; includes voltage converter losses).
 */

#ifndef HWS_CRAY_PM_COUNTERS_HARDWARE_SAMPLER_HPP_
#define HWS_CRAY_PM_COUNTERS_HARDWARE_SAMPLER_HPP_
#pragma once

#include "hws/cray_pm_counters/pm_counters_samples.hpp"  // hws::{cray_pm_counters_general_samples, cray_pm_counters_power_samples}
#include "hws/hardware_sampler.hpp"                      // hws::hardware_sampler
#include "hws/sample_category.hpp"                       // hws::sample_category

#include "fmt/ostream.h"  // fmt::formatter, fmt::ostream_formatter

#include <chrono>         // std::chrono::milliseconds, std::chrono_literals namespace
#include <filesystem>     // std::filesystem::path
#include <iosfwd>         // std::ostream forward declaration
#include <optional>       // std::optional
#include <unordered_map>  // std::unordered_map

namespace hws {

using namespace std::chrono_literals;

/**
 * @brief A hardware sampler for whole-node power/energy using Cray/HPE's `/sys/cray/pm_counters` sysfs interface.
 * @details Since `pm_counters` is a per-node (not per-device) sysfs interface, only a single instance of this
 *          sampler exists per node (mirroring `hws::cpu_hardware_sampler`), even on nodes with an accelerator
 *          (whose "accel energy"/"accel power" counters `pm_counters` also exposes). In MPI `whole_node` mode
 *          only the node-local rank 0 creates this sampler; in `per_rank` mode (and the non-MPI, single-process
 *          case), every rank on a node creates its own instance, so the same node-wide ground truth is sampled and
 *          reported redundantly once per rank - this mirrors `hws::cpu_hardware_sampler`'s existing behavior.
 */
class cray_pm_counters_hardware_sampler : public hardware_sampler {
  public:
    /**
     * @brief Construct a new pm_counters hardware sampler with the default sampling interval.
     * @param[in] category the sample categories that are enabled for hardware sampling (default: all)
     */
    explicit cray_pm_counters_hardware_sampler(sample_category category = sample_category::all);
    /**
     * @brief Construct a new pm_counters hardware sampler with the @p sampling_interval.
     * @param[in] sampling_interval the used sampling interval
     * @param[in] category the sample categories that are enabled for hardware sampling (default: all)
     */
    explicit cray_pm_counters_hardware_sampler(std::chrono::milliseconds sampling_interval, sample_category category = sample_category::all);

    /**
     * @brief Delete the copy-constructor (already implicitly deleted due to the base class's std::atomic member).
     */
    cray_pm_counters_hardware_sampler(const cray_pm_counters_hardware_sampler &) = delete;
    /**
     * @brief Delete the move-constructor (already implicitly deleted due to the base class's std::atomic member).
     */
    cray_pm_counters_hardware_sampler(cray_pm_counters_hardware_sampler &&) noexcept = delete;
    /**
     * @brief Delete the copy-assignment operator (already implicitly deleted due to the base class's std::atomic member).
     */
    cray_pm_counters_hardware_sampler &operator=(const cray_pm_counters_hardware_sampler &) = delete;
    /**
     * @brief Delete the move-assignment operator (already implicitly deleted due to the base class's std::atomic member).
     */
    cray_pm_counters_hardware_sampler &operator=(cray_pm_counters_hardware_sampler &&) noexcept = delete;

    /**
     * @brief Destruct the pm_counters hardware sampler. If the sampler is still running, stops it.
     */
    ~cray_pm_counters_hardware_sampler() override;

    /**
     * @brief Return the general pm_counters samples of this hardware sampler.
     * @return the general pm_counters samples (`[[nodiscard]]`)
     */
    [[nodiscard]] const cray_pm_counters_general_samples &general_samples() const noexcept { return general_samples_; }

    /**
     * @brief Return the power related pm_counters samples of this hardware sampler.
     * @return the power related pm_counters samples (`[[nodiscard]]`)
     */
    [[nodiscard]] const cray_pm_counters_power_samples &power_samples() const noexcept { return power_samples_; }

    /**
     * @copydoc hws::hardware_sampler::device_identification
     */
    [[nodiscard]] std::string device_identification() const final;

    /**
     * @copydoc hws::hardware_sampler::samples_only_as_yaml_string() const
     */
    [[nodiscard]] std::string samples_only_as_yaml_string() const final;

  private:
    /**
     * @copydoc hws::hardware_sampler::sampling_loop
     */
    void sampling_loop() final;

    /**
     * @brief Read every discovered pm_counters file once and append the readings to `general_samples_`/`power_samples_`.
     * @details Called both for the very first sample and on every subsequent tick of the sampling loop - all three
     *          categories (general, energy, power) are re-read every tick, see `cray_pm_counters_general_samples`.
     */
    void sample_once();

    /**
     * @brief Derive the hardware's own update period from the already-sampled `raw_scan_hz` metadata.
     * @return the hardware tick period, or `std::nullopt` if `raw_scan_hz` wasn't found/couldn't be parsed (`[[nodiscard]]`)
     */
    [[nodiscard]] std::optional<std::chrono::milliseconds> hardware_tick_period() const;

    /// The general pm_counters samples.
    cray_pm_counters_general_samples general_samples_{};
    /// The power related pm_counters samples.
    cray_pm_counters_power_samples power_samples_{};

    /// The full sysfs path of every general metadata file discovered at sampling start, keyed by its `general_samples().get_metadata()` map key.
    std::unordered_map<std::string, std::filesystem::path> general_paths_{};
    /// The full sysfs path of every energy counter discovered at sampling start, keyed by its `power_samples().get_energy_counters()` map key.
    std::unordered_map<std::string, std::filesystem::path> energy_counter_paths_{};
    /// The full sysfs path of every power counter discovered at sampling start, keyed by its `power_samples().get_power_counters()` map key.
    std::unordered_map<std::string, std::filesystem::path> power_counter_paths_{};
};

/**
 * @brief Output all pm_counters samples gathered by the @p sampler to the given output-stream @p out.
 * @details Sets `std::ios_base::failbit` if the @p sampler is still sampling.
 * @param[in,out] out the output-stream to write the pm_counters samples to
 * @param[in] sampler the pm_counters hardware sampler
 * @return the output-stream
 */
std::ostream &operator<<(std::ostream &out, const cray_pm_counters_hardware_sampler &sampler);

}  // namespace hws

/// @cond Doxygen_suppress

template <>
struct fmt::formatter<hws::cray_pm_counters_hardware_sampler> : fmt::ostream_formatter { };

/// @endcond

#endif  // HWS_CRAY_PM_COUNTERS_HARDWARE_SAMPLER_HPP_
