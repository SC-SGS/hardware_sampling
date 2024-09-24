/**
 * @file
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Defines the samples used with Level Zero.
 */

#ifndef HARDWARE_SAMPLING_GPU_INTEL_LEVEL_ZERO_SAMPLES_HPP_
#define HARDWARE_SAMPLING_GPU_INTEL_LEVEL_ZERO_SAMPLES_HPP_
#pragma once

#include "hardware_sampling/utility.hpp"  // HWS_SAMPLE_STRUCT_FIXED_MEMBER, HWS_SAMPLE_STRUCT_SAMPLING_MEMBER

#include "fmt/ostream.h"  // fmt::formatter, fmt::ostream_formatter

#include <cstdint>        // std::uint64_t, std::int32_t
#include <iosfwd>         // std::ostream forward declaration
#include <optional>       // std::optional
#include <string>         // std::string
#include <unordered_map>  // std::unordered_map
#include <vector>         // std::vector

namespace hws {

//*************************************************************************************************************************************//
//                                                           general samples                                                           //
//*************************************************************************************************************************************//

/**
 * @brief Wrapper class for all general Level Zero hardware samples.
 */
class level_zero_general_samples {
    // befriend hardware sampler class
    friend class gpu_intel_hardware_sampler;

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

    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::string, byte_order)          // the byte order (e.g., little/big endian)
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::string, vendor_id)           // the vendor ID
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::string, name)                // the model name of the device
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::vector<std::string>, flags)  // potential GPU flags (e.g. integrated device)

    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::string, standby_mode)          // the enabled standby mode (power saving or never)
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::uint32_t, num_threads_per_eu)  // the number of threads per EU unit
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::uint32_t, eu_simd_width)       // the physical EU unit SIMD width
};

/**
 * @brief Output the general @p samples to the given output-stream @p out.
 * @details In contrast to `level_zero_general_samples::generate_yaml_string()`, outputs **all** general hardware samples, even if not supported by the current device (default initialized value).
 * @param[in,out] out the output-stream to write the general hardware samples to
 * @param[in] samples the Level Zero general samples
 * @return the output-stream
 */
std::ostream &operator<<(std::ostream &out, const level_zero_general_samples &samples);

//*************************************************************************************************************************************//
//                                                            clock samples                                                            //
//*************************************************************************************************************************************//

/**
 * @brief Wrapper class for all clock related Level Zero hardware samples.
 */
class level_zero_clock_samples {
    // befriend hardware sampler class
    friend class gpu_intel_hardware_sampler;

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

    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, clock_frequency_min)                              // the minimum possible GPU clock frequency in MHz
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, clock_frequency_max)                              // the maximum possible GPU clock frequency in MHz
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, memory_clock_frequency_min)                       // the minimum possible memory clock frequency in MHz
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, memory_clock_frequency_max)                       // the maximum possible memory clock frequency in MHz
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::vector<double>, available_clock_frequencies)         // the available GPU clock frequencies in MHz (slowest to fastest)
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::vector<double>, available_memory_clock_frequencies)  // the available memory clock frequencies in MHz (slowest to fastest)

    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, clock_frequency)              // the current GPU frequency in MHz
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, memory_clock_frequency)       // the current memory frequency in MHz
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(std::string, throttle_reason)         // the current GPU frequency throttle reason
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(std::string, memory_throttle_reason)  // the current memory frequency throttle reason
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, frequency_limit_tdp)          // the current maximum allowed GPU frequency based on the TDP limit in MHz
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, memory_frequency_limit_tdp)   // the current maximum allowed memory frequency based on the TDP limit in MHz
};

/**
 * @brief Output the clock related @p samples to the given output-stream @p out.
 * @details In contrast to `level_zero_clock_samples::generate_yaml_string()`, outputs **all** clock related hardware samples, even if not supported by the current device (default initialized value).
 * @param[in,out] out the output-stream to write the clock related hardware samples to
 * @param[in] samples the Level Zero clock related samples
 * @return the output-stream
 */
std::ostream &operator<<(std::ostream &out, const level_zero_clock_samples &samples);

//*************************************************************************************************************************************//
//                                                            power samples                                                            //
//*************************************************************************************************************************************//

/**
 * @brief Wrapper class for all power related Level Zero hardware samples.
 */
class level_zero_power_samples {
    // befriend hardware sampler class
    friend class gpu_intel_hardware_sampler;

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

    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, power_enforced_limit)         // the actually enforced power limit (W), may be different from power management limit if external limiters are set
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::string, power_measurement_type)  // the type of the power readings
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(bool, power_management_mode)          // true if power management limits are enabled

    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, power_usage)                     // the current power draw of the GPU in W (calculated from power_total_energy_consumption)
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, power_total_energy_consumption)  // the total power consumption since the last driver reload in J
};

/**
 * @brief Output the power related @p samples to the given output-stream @p out.
 * @details In contrast to `level_zero_power_samples::generate_yaml_string()`, outputs **all** power related hardware samples, even if not supported by the current device (default initialized value).
 * @param[in,out] out the output-stream to write the power related hardware samples to
 * @param[in] samples the Level Zero power related samples
 * @return the output-stream
 */
std::ostream &operator<<(std::ostream &out, const level_zero_power_samples &samples);

//*************************************************************************************************************************************//
//                                                            memory samples                                                           //
//*************************************************************************************************************************************//

/**
 * @brief Wrapper class for all memory related Level Zero hardware samples.
 */
class level_zero_memory_samples {
    // befriend hardware sampler class
    friend class gpu_intel_hardware_sampler;

    /**
     * @brief The map type used if the number of potential Level Zero domains is unknown at compile time.
     * @tparam T the mapped type
     */
    template <typename T>
    using map_type = std::unordered_map<std::string, T>;

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

    HWS_SAMPLE_STRUCT_FIXED_MEMBER(map_type<std::uint64_t>, memory_total)          // the total memory size of the different memory modules in Bytes
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(map_type<std::uint64_t>, visible_memory_total)  // the total allocatable memory size of the different memory modules in Bytes
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(map_type<std::string>, memory_location)         // the location of the different memory modules (system or device)
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::int32_t, num_pcie_lanes_max)               // the maximum PCIe lane width
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::int32_t, pcie_link_generation_max)         // the maximum PCIe generation
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::int64_t, pcie_link_speed_max)              // the maximum PCIe bandwidth in MBPS
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(map_type<std::int32_t>, memory_bus_width)       // the bus width of the different memory modules
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(map_type<std::int32_t>, memory_num_channels)    // the number of memory channels of the different memory modules

    HWS_SAMPLE_STRUCT_FIXED_MEMBER(map_type<std::vector<std::uint64_t>>, memory_free)  // the currently free memory of the different memory modules in Bytes
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(map_type<std::vector<std::uint64_t>>, memory_used)  // the currently used memory of the different memory modules in Bytes
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(std::int32_t, num_pcie_lanes)                    // the current PCIe lane width
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(std::int32_t, pcie_link_generation)              // the current PCIe generation
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(std::int64_t, pcie_link_speed)                   // the current PCIe bandwidth in bytes/sec
};

/**
 * @brief Output the memory related @p samples to the given output-stream @p out.
 * @details In contrast to `level_zero_memory_samples::generate_yaml_string()`, outputs **all** memory related hardware samples, even if not supported by the current device (default initialized value).
 * @param[in,out] out the output-stream to write the memory related hardware samples to
 * @param[in] samples the Level Zero memory related samples
 * @return the output-stream
 */
std::ostream &operator<<(std::ostream &out, const level_zero_memory_samples &samples);

//*************************************************************************************************************************************//
//                                                         temperature samples                                                         //
//*************************************************************************************************************************************//

/**
 * @brief Wrapper class for all temperature related Level Zero hardware samples.
 */
class level_zero_temperature_samples {
    // befriend hardware sampler class
    friend class gpu_intel_hardware_sampler;

  public:
    /**
     * @brief Checks whether any temperature related hardware sample is present.
     * @return `true` if any temperature related hardware sample is, otherwise `false`.
     */
    [[nodiscard]] bool has_samples() const;
    /**
     * @brief Assemble the YAML string containing all available general hardware samples.
     * @details Hardware samples that are not supported by the current device are omitted in the YAML output.
     *          Returns an empty string if `has_samples()` returns `false`.
     * @return the YAML string (`[[nodiscard]]`)
     */
    [[nodiscard]] std::string generate_yaml_string() const;

    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::uint32_t, num_fans)         // the number of fans
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::int32_t, fan_speed_max)     // the maximum fan speed the user can set in RPM
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, temperature_max)         // the maximum GPU temperature in °C
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, memory_temperature_max)  // the maximum memory temperature in °C
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, global_temperature_max)  // the maximum global temperature in °C

    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, fan_speed_percentage)  // the current intended fan speed in %
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, temperature)           // the temperature of the GPU in °C
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, memory_temperature)    // the temperature of the memory in °C
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, global_temperature)    // the global temperature in °C
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, psu_temperature)       // the temperature of the PSU in °C
};

/**
 * @brief Output the temperature related @p samples to the given output-stream @p out.
 * @details In contrast to `level_zero_temperature_samples::generate_yaml_string()`, outputs **all** temperature related hardware samples, even if not supported by the current device (default initialized value).
 * @param[in,out] out the output-stream to write the temperature related hardware samples to
 * @param[in] samples the Level Zero temperature related samples
 * @return the output-stream
 */
std::ostream &operator<<(std::ostream &out, const level_zero_temperature_samples &samples);

}  // namespace hws

template <>
struct fmt::formatter<hws::level_zero_general_samples> : fmt::ostream_formatter { };

template <>
struct fmt::formatter<hws::level_zero_clock_samples> : fmt::ostream_formatter { };

template <>
struct fmt::formatter<hws::level_zero_power_samples> : fmt::ostream_formatter { };

template <>
struct fmt::formatter<hws::level_zero_memory_samples> : fmt::ostream_formatter { };

template <>
struct fmt::formatter<hws::level_zero_temperature_samples> : fmt::ostream_formatter { };

#endif  // HARDWARE_SAMPLING_GPU_INTEL_LEVEL_ZERO_SAMPLES_HPP_
