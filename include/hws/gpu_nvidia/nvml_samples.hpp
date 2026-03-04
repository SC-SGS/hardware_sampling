/**
 * @file
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Defines the samples used with NVML.
 */

#ifndef HWS_GPU_NVIDIA_NVML_SAMPLES_HPP_
#define HWS_GPU_NVIDIA_NVML_SAMPLES_HPP_
#pragma once

#include "hws/utility.hpp"  // HWS_SAMPLE_STRUCT_FIXED_MEMBER, HWS_SAMPLE_STRUCT_SAMPLING_MEMBER

#include "fmt/ostream.h"  // fmt::formatter, fmt::ostream_formatter

#include <iosfwd>    // std::ostream forward declaration
#include <map>       // std::map
#include <optional>  // std::optional
#include <string>    // std::string
#include <vector>    // std::vector

namespace hws {

//*************************************************************************************************************************************//
//                                                           general samples                                                           //
//*************************************************************************************************************************************//

/**
 * @brief Wrapper class for all general NVML hardware samples.
 */
class nvml_general_samples {
    // befriend hardware sampler class
    friend class gpu_nvidia_hardware_sampler;

  public:
    /**
     * @brief Checks whether any general hardware sample is present.
     * @return `true` if any general hardware sample is, otherwise `false`.
     */
    [[nodiscard]] bool has_samples() const;
    /**
     * @brief Assemble the YAML string containing all available general hardware samples.
     * @details Hardware samples that are not supported by the current device are omitted in the YAML output.
     *          Returns an empty string if `has_samples()` returns `false`.
     * @return the YAML string (`[[nodiscard]]`)
     */
    [[nodiscard]] std::string generate_yaml_string() const;

    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::string, architecture)  // the architecture name of the device
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::string, byte_order)    // the byte order (e.g., little/big endian)
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(unsigned int, num_cores)    // the number of CUDA cores
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::string, vendor_id)     // the vendor ID
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::string, name)          // the name of the device
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(bool, persistence_mode)     // the persistence mode: if true, the driver is always loaded reducing the latency for the first API call

    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(unsigned int, compute_utilization)  // the GPU compute utilization in percent
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(unsigned int, memory_utilization)   // the GPU memory utilization in percent
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(int, performance_level)             // the performance state: 0 - 15 where 0 is the maximum performance and 15 the minimum performance
};

/**
 * @brief Output the general @p samples to the given output-stream @p out.
 * @details In contrast to `nvml_general_samples::generate_yaml_string()`, outputs **all** general hardware samples, even if not supported by the current device (default initialized value).
 * @param[in,out] out the output-stream to write the general hardware samples to
 * @param[in] samples the NVML general samples
 * @return the output-stream
 */
std::ostream &operator<<(std::ostream &out, const nvml_general_samples &samples);

//*************************************************************************************************************************************//
//                                                            clock samples                                                            //
//*************************************************************************************************************************************//

/**
 * @brief Wrapper class for all clock related NVML hardware samples.
 */
class nvml_clock_samples {
    // befriend hardware sampler class
    friend class gpu_nvidia_hardware_sampler;

    /// The map type used to map the available clock frequencies to a specific memory frequency.
    using map_type = std::map<double, std::vector<double>>;

  public:
    /**
     * @brief Checks whether any clock related hardware sample is present.
     * @return `true` if any clock related hardware sample is, otherwise `false`.
     */
    [[nodiscard]] bool has_samples() const;
    /**
     * @brief Assemble the YAML string containing all available general hardware samples.
     * @details Hardware samples that are not supported by the current device are omitted in the YAML output.
     *          Returns an empty string if `has_samples()` returns `false`.
     * @return the YAML string (`[[nodiscard]]`)
     */
    [[nodiscard]] std::string generate_yaml_string() const;

    HWS_SAMPLE_STRUCT_FIXED_MEMBER(bool, auto_boosted_clock_enabled)                         // true if clock boosting is currently enabled
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, clock_frequency_min)                              // the minimum possible graphics clock frequency in MHz
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, clock_frequency_max)                              // the maximum possible graphics clock frequency in MHz
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, memory_clock_frequency_min)                       // the minimum possible memory clock frequency in MHz
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, memory_clock_frequency_max)                       // the maximum possible memory clock frequency in MHz
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, sm_clock_frequency_max)                           // the maximum possible SM clock frequency in MHz
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(map_type, available_clock_frequencies)                    // the available clock frequencies in MHz, based on a memory clock frequency (slowest to fastest)
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::vector<double>, available_memory_clock_frequencies)  // the available memory clock frequencies in MHz (slowest to fastest)

    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, clock_frequency)              // the current graphics clock frequency in MHz
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, memory_clock_frequency)       // the current memory clock frequency in MHz
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, sm_clock_frequency)           // the current SM clock frequency in Mhz
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(unsigned long long, throttle_reason)  // the reason the GPU clock throttled (as bitmask)
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(std::string, throttle_reason_string)  // the reason the GPU clock throttled (as string)
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(bool, auto_boosted_clock)             // true if the clocks are currently auto boosted
};

/**
 * @brief Output the clock related @p samples to the given output-stream @p out.
 * @details In contrast to `nvml_clock_samples::generate_yaml_string()`, outputs **all** clock related hardware samples, even if not supported by the current device (default initialized value).
 * @param[in,out] out the output-stream to write the clock related hardware samples to
 * @param[in] samples the NVML clock related samples
 * @return the output-stream
 */
std::ostream &operator<<(std::ostream &out, const nvml_clock_samples &samples);

//*************************************************************************************************************************************//
//                                                            power samples                                                            //
//*************************************************************************************************************************************//

/**
 * @brief Wrapper class for all power related NVML hardware samples.
 */
class nvml_power_samples {
    // befriend hardware sampler class
    friend class gpu_nvidia_hardware_sampler;

  public:
    /**
     * @brief Checks whether any power related hardware sample is present.
     * @return `true` if any power related hardware sample is, otherwise `false`.
     */
    [[nodiscard]] bool has_samples() const;
    /**
     * @brief Assemble the YAML string containing all available general hardware samples.
     * @details Hardware samples that are not supported by the current device are omitted in the YAML output.
     *          Returns an empty string if `has_samples()` returns `false`.
     * @return the YAML string (`[[nodiscard]]`)
     */
    [[nodiscard]] std::string generate_yaml_string() const;

    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, power_management_limit)              // if the GPU draws more power (W) than the power management limit, the GPU may throttle
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, power_enforced_limit)                // the actually enforced power limit (W), may be different from power management limit if external limiters are set
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::string, power_measurement_type)         // the type of the power readings: either current power draw or average power draw
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(bool, power_management_mode)                 // true if power management algorithms are supported and active
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::vector<int>, available_power_profiles)  // a list of the available power profiles

    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, power_usage)                     // the current power draw of the GPU and its related circuity (e.g., memory) in W
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, power_total_energy_consumption)  // the total power consumption since the last driver reload in J
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(int, power_profile)                      // the current GPU power state: 0 - 15 where 0 is the maximum power and 15 the minimum power; 32 indicates unknown
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, system_power_usage)                      // total system power for module based NVIDIA systems like GH200; includes all chips on a module (e.g. CPU+GPU or CPU+CPU)
};

/**
 * @brief Output the power related @p samples to the given output-stream @p out.
 * @details In contrast to `nvml_power_samples::generate_yaml_string()`, outputs **all** power related hardware samples, even if not supported by the current device (default initialized value).
 * @param[in,out] out the output-stream to write the power related hardware samples to
 * @param[in] samples the NVML power related samples
 * @return the output-stream
 */
std::ostream &operator<<(std::ostream &out, const nvml_power_samples &samples);

//*************************************************************************************************************************************//
//                                                            memory samples                                                           //
//*************************************************************************************************************************************//

/**
 * @brief Wrapper class for all memory related NVML hardware samples.
 */
class nvml_memory_samples {
    // befriend hardware sampler class
    friend class gpu_nvidia_hardware_sampler;

  public:
    /**
     * @brief Checks whether any memory related hardware sample is present.
     * @return `true` if any memory related hardware sample is, otherwise `false`.
     */
    [[nodiscard]] bool has_samples() const;
    /**
     * @brief Assemble the YAML string containing all available general hardware samples.
     * @details Hardware samples that are not supported by the current device are omitted in the YAML output.
     *          Returns an empty string if `has_samples()` returns `false`.
     * @return the YAML string (`[[nodiscard]]`)
     */
    [[nodiscard]] std::string generate_yaml_string() const;

    HWS_SAMPLE_STRUCT_FIXED_MEMBER(unsigned long, memory_total)             // the total available memory in Byte
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(unsigned int, num_pcie_lanes_max)        // the maximum number of PCIe lanes
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(unsigned int, pcie_link_generation_max)  // the maximum PCIe link generation (e.g., PCIe 4.0, PCIe 5.0, etc.)
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(unsigned int, pcie_link_speed_max)       // the maximum PCIe link speed in MBPS
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(unsigned int, memory_bus_width)          // the memory bus with in Bit

    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(unsigned long long, memory_used)     // the currently used memory in Byte
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(unsigned long long, memory_free)     // the currently free memory in Byte
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(unsigned int, num_pcie_lanes)        // the current PCIe link width (e.g., x16, x8, x4, etc)
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(unsigned int, pcie_link_generation)  // the current PCIe link generation (may change during runtime to save energy)
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(unsigned int, pcie_link_speed)       // the current PCIe link speed in MBPS
};

/**
 * @brief Output the memory related @p samples to the given output-stream @p out.
 * @details In contrast to `nvml_memory_samples::generate_yaml_string()`, outputs **all** memory related hardware samples, even if not supported by the current device (default initialized value).
 * @param[in,out] out the output-stream to write the memory related hardware samples to
 * @param[in] samples the NVML memory related samples
 * @return the output-stream
 */
std::ostream &operator<<(std::ostream &out, const nvml_memory_samples &samples);

//*************************************************************************************************************************************//
//                                                         temperature samples                                                         //
//*************************************************************************************************************************************//

/**
 * @brief Wrapper class for all temperature related NVML hardware samples.
 */
class nvml_temperature_samples {
    // befriend hardware sampler class
    friend class gpu_nvidia_hardware_sampler;

  public:
    /**
     * @brief Checks whether any temperature related hardware sample is present.
     * @return `true` if any temperature related hardware sample is, otherwise `false`.
     */
    [[nodiscard]] bool has_samples() const;
    /**
     * @brief Assemble the YAML string containing all available general hardware samples.
     * @details Hardware samples that are not supported by the current device are omitted in the YAML output.
     * @return the YAML string (`[[nodiscard]]`)
     */
    [[nodiscard]] std::string generate_yaml_string() const;

    HWS_SAMPLE_STRUCT_FIXED_MEMBER(unsigned int, num_fans)          // the number of fans (if any)
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(unsigned int, fan_speed_min)     // the minimum fan speed the user can set in %
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(unsigned int, fan_speed_max)     // the maximum fan speed the user can set in %
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, temperature_max)         // the maximum graphics temperature threshold in °C
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, memory_temperature_max)  // the maximum memory temperature threshold in °C

    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, fan_speed_percentage)  // the current intended fan speed in %
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, temperature)           // the current GPU temperature in °C
};

/**
 * @brief Output the temperature related @p samples to the given output-stream @p out.
 * @details In contrast to `nvml_temperature_samples::generate_yaml_string()`, outputs **all** temperature related hardware samples, even if not supported by the current device (default initialized value).
 * @param[in,out] out the output-stream to write the temperature related hardware samples to
 * @param[in] samples the NVML temperature related samples
 * @return the output-stream
 */
std::ostream &operator<<(std::ostream &out, const nvml_temperature_samples &samples);

}  // namespace hws

/// @cond Doxygen_suppress

template <>
struct fmt::formatter<hws::nvml_general_samples> : fmt::ostream_formatter { };

template <>
struct fmt::formatter<hws::nvml_clock_samples> : fmt::ostream_formatter { };

template <>
struct fmt::formatter<hws::nvml_power_samples> : fmt::ostream_formatter { };

template <>
struct fmt::formatter<hws::nvml_memory_samples> : fmt::ostream_formatter { };

template <>
struct fmt::formatter<hws::nvml_temperature_samples> : fmt::ostream_formatter { };

/// @endcond

#endif  // HWS_GPU_NVIDIA_NVML_SAMPLES_HPP_
