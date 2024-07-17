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

#include "hardware_sampling/utility.hpp"  // HWS_SAMPLE_STRUCT_FIXED_MEMBER, HWS_SAMPLE_STRUCT_SAMPLING_MEMBER, hws::detail::ostream_formatter

#include <cstdint>        // std::uint64_t, std::int32_t
#include <format>         // std::format
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
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::string, name)                  // the model name of the device
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
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, clock_gpu_min)                      // the minimum possible GPU clock frequency in MHz
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, clock_gpu_max)                      // the maximum possible GPU clock frequency in MHz
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::vector<double>, available_clocks_gpu)  // the available GPU clock frequencies in MHz (slowest to fastest)
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, clock_mem_min)                      // the minimum possible memory clock frequency in MHz
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, clock_mem_max)                      // the maximum possible memory clock frequency in MHz
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::vector<double>, available_clocks_mem)  // the available memory clock frequencies in MHz (slowest to fastest)

    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, tdp_frequency_limit_gpu)  // the current maximum allowed GPU frequency based on the TDP limit in MHz
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, clock_gpu)                // the current GPU frequency in MHz
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(int, throttle_reason_gpu)         // the current GPU frequency throttle reason
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, tdp_frequency_limit_mem)  // the current maximum allowed memory frequency based on the TDP limit in MHz
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, clock_mem)                // the current memory frequency in MHz
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(int, throttle_reason_mem)         // the current memory frequency throttle reason
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
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(bool, energy_threshold_enabled)  // true if the energy threshold is enabled
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, energy_threshold)        // the energy threshold in J

    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(std::uint64_t, power_total_energy_consumption)  // the total power consumption since the last driver reload in mJ
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
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(map_type<std::uint64_t>, memory_total)              // the total memory size of the different memory modules in Bytes
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(map_type<std::uint64_t>, allocatable_memory_total)  // the total allocatable memory size of the different memory modules in Bytes
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::int64_t, pcie_link_max_speed)                  // the maximum PCIe bandwidth in bytes/sec
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::int32_t, pcie_max_width)                       // the PCIe lane width
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::int32_t, max_pcie_link_generation)             // the PCIe generation
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(map_type<std::int32_t>, bus_width)                  // the bus width of the different memory modules
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(map_type<std::int32_t>, num_channels)               // the number of memory channels of the different memory modules
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(map_type<std::string>, location)                    // the location of the different memory modules (system or device)

    HWS_SAMPLE_STRUCT_FIXED_MEMBER(map_type<std::vector<std::uint64_t>>, memory_free)  // the currently free memory of the different memory modules in Bytes
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(std::int64_t, pcie_link_speed)                   // the current PCIe bandwidth in bytes/sec
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(std::int32_t, pcie_link_width)                   // the current PCIe lane width
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(std::int32_t, pcie_link_generation)              // the current PCIe generation
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

    /**
     * @brief The map type used if the number of potential Level Zero domains is unknown at compile time.
     * @tparam T the mapped type
     */
    template <typename T>
    using map_type = std::unordered_map<std::string, T>;

  public:
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(map_type<double>, temperature_max)  // the maximum temperature for the sensor in °C

    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(std::int32_t, temperature_psu)            // the temperature of the PSU in °C
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(map_type<std::vector<double>>, temperature)  // the current temperature for the sensor in °C
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
struct std::formatter<hws::level_zero_general_samples> : hws::detail::ostream_formatter { };

template <>
struct std::formatter<hws::level_zero_clock_samples> : hws::detail::ostream_formatter { };

template <>
struct std::formatter<hws::level_zero_power_samples> : hws::detail::ostream_formatter { };

template <>
struct std::formatter<hws::level_zero_memory_samples> : hws::detail::ostream_formatter { };

template <>
struct std::formatter<hws::level_zero_temperature_samples> : hws::detail::ostream_formatter { };

#endif  // HARDWARE_SAMPLING_GPU_INTEL_LEVEL_ZERO_SAMPLES_HPP_
