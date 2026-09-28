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

#include <cstddef>      // std::size_t
#include <cstdint>      // std::uint64_t
#include <filesystem>   // std::filesystem::path
#include <optional>     // std::optional
#include <string>       // std::string
#include <string_view>  // std::string_view
#include <utility>      // std::pair
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

/**
 * @brief Extract the accelerator index from an energy or power counter map key, e.g. `"accel2_energy"` -> `2`.
 * @param[in] key the map key to check, as returned by `pm_counter_key`
 * @return the accelerator index, or `std::nullopt` if @p key isn't of the form `accel<N>_energy`/`accel<N>_power`
 *         (`[[nodiscard]]`)
 */
[[nodiscard]] std::optional<int> accel_index_from_counter_key(const std::string &key);

/**
 * @brief Extract the sorted, deduplicated set of accelerator indices referenced by @p counter_keys.
 * @details Intended to be called with the keys of an already-sampled `cray_pm_counters_power_samples`'s
 *          `energy_counters`/`power_counters` map, to answer "how many `accel[i]` counters did pm_counters
 *          actually expose on this node" without re-scanning the filesystem.
 * @param[in] counter_keys the energy or power counter map keys to scan, as returned by `pm_counter_key`
 * @return the sorted accelerator indices found (`[[nodiscard]]`)
 */
[[nodiscard]] std::vector<int> accel_indices_from_counter_keys(const std::vector<std::string> &counter_keys);

/**
 * @brief Guess which pm_counters `accel[i]` a given AMD GPU (identified by its PCI bus ID) corresponds to.
 * @details UNVERIFIED heuristic: assumes `accel[i]` numbers accelerators in ascending PCI bus address order among
 *          all physically present AMD GPUs - no HPE documentation defines this correspondence, so this should be
 *          confirmed empirically (e.g. drive load on a single visible GPU and observe which `accel[i]_power`
 *          reacts) before being relied on for analysis. See `hws::system_hardware_sampler::device_correlation_hints_as_yaml_string()`,
 *          the only caller, for how this is surfaced to users.
 * @param[in] pci_bus_id the PCI bus ID of the AMD GPU to guess an accel index for
 * @param[in] physical_pci_bus_ids_sorted every physically present AMD GPU's PCI bus ID, sorted ascending (as
 *            returned by `hws::detail::enumerate_all_amd_gpu_pci_bus_ids()`)
 * @param[in] accel_indices_sorted the accelerator indices pm_counters exposed, sorted ascending (as returned by
 *            `cray_pm_counters_hardware_sampler::discovered_accel_indices()`)
 * @return the guessed accel index, or `std::nullopt` if @p pci_bus_id isn't found in @p physical_pci_bus_ids_sorted
 *         or the two lists don't have the same size (a topology count mismatch means the guess isn't safe to make,
 *         e.g. under a cgroup-isolated partial-node allocation) (`[[nodiscard]]`)
 */
[[nodiscard]] std::optional<int> guess_accel_index(const std::string &pci_bus_id,
                                                    const std::vector<std::string> &physical_pci_bus_ids_sorted,
                                                    const std::vector<int> &accel_indices_sorted);

/**
 * @brief Build the YAML sub-block (under a `gpu_vendors:` mapping) describing one GPU vendor's accel[i]
 *        correlation guesses, e.g. for `"amd"` or `"nvidia"`.
 * @details Vendor-agnostic: the caller is responsible for gathering @p visible_devices and
 *          @p physical_pci_bus_ids_sorted using the right backend (`hws::detail::amd_device_pci_bus_id()`/
 *          `enumerate_all_amd_gpu_pci_bus_ids()` or their `nvidia_*` counterparts). Every entry in
 *          @p physical_pci_bus_ids_sorted and @p accel_indices_sorted is assumed to belong to @p vendor alone -
 *          don't call this with a mixed-vendor physical topology.
 * @param[in] vendor the vendor name to use as the YAML mapping key (e.g. `"amd"`, `"nvidia"`)
 * @param[in] visible_devices every visible GPU sampler of this vendor, as (local device index, PCI bus ID) pairs
 * @param[in] physical_pci_bus_ids_sorted every physically present GPU of this vendor's PCI bus ID, sorted ascending
 * @param[in] accel_indices_sorted the accelerator indices pm_counters exposed, sorted ascending
 * @return the YAML sub-block, indented to sit directly under a `  gpu_vendors:\n` key (`[[nodiscard]]`)
 */
[[nodiscard]] std::string accel_correlation_yaml_block(const std::string &vendor,
                                                        const std::vector<std::pair<std::size_t, std::string>> &visible_devices,
                                                        const std::vector<std::string> &physical_pci_bus_ids_sorted,
                                                        const std::vector<int> &accel_indices_sorted);

}  // namespace hws::detail

#endif  // HWS_CRAY_PM_COUNTERS_UTILITY_HPP_
