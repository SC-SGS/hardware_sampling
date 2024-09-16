/**
 * @file
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Defines the samples used with ROCm SMI.
 */

#ifndef HARDWARE_SAMPLING_GPU_AMD_ROCM_SMI_SAMPLES_HPP_
#define HARDWARE_SAMPLING_GPU_AMD_ROCM_SMI_SAMPLES_HPP_
#pragma once

#include "hardware_sampling/utility.hpp"  // HWS_SAMPLE_STRUCT_FIXED_MEMBER, HWS_SAMPLE_STRUCT_SAMPLING_MEMBER

#include "fmt/ostream.h"  // fmt::formatter, fmt::ostream_formatter

#include <cstdint>   // std::uint64_t, std::int64_t, std::uint32_t
#include <iosfwd>    // std::ostream forward declaration
#include <optional>  // std::optional
#include <string>    // std::string
#include <vector>    // std::vector

namespace hws {

//*************************************************************************************************************************************//
//                                                           general samples                                                           //
//*************************************************************************************************************************************//

/**
 * @brief Wrapper class for all general ROCm SMI hardware samples.
 */
class rocm_smi_general_samples {
    // befriend hardware sampler class
    friend class gpu_amd_hardware_sampler;

  public:
    /**
     * @brief Assemble the YAML string containing all available general hardware samples.
     * @details Hardware samples that are not supported by the current device are omitted in the YAML output.
     * @return the YAML string (`[[nodiscard]]`)
     */
    [[nodiscard]] std::string generate_yaml_string() const;

    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::string, architecture)  // the architecture name of the device
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::string, byte_order)    // the byte order (e.g., little/big endian)
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::string, vendor_id)     // the vendor ID
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::string, name)          // the name of the device

    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(std::uint32_t, compute_utilization)  // the GPU compute utilization in percent
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(std::uint32_t, memory_utilization)   // the GPU memory utilization in percent
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(std::string, performance_level)              // the performance level: one of rsmi_dev_perf_level_t
};

/**
 * @brief Output the general @p samples to the given output-stream @p out.
 * @details In contrast to `rocm_smi_general_samples::generate_yaml_string()`, outputs **all** general hardware samples, even if not supported by the current device (default initialized value).
 * @param[in,out] out the output-stream to write the general hardware samples to
 * @param[in] samples the ROCm SMI general samples
 * @return the output-stream
 */
std::ostream &operator<<(std::ostream &out, const rocm_smi_general_samples &samples);

//*************************************************************************************************************************************//
//                                                            clock samples                                                            //
//*************************************************************************************************************************************//

/**
 * @brief Wrapper class for all clock related ROCm SMI hardware samples.
 */
class rocm_smi_clock_samples {
    // befriend hardware sampler class
    friend class gpu_amd_hardware_sampler;

  public:
    /**
     * @brief Assemble the YAML string containing all available general hardware samples.
     * @details Hardware samples that are not supported by the current device are omitted in the YAML output.
     * @return the YAML string (`[[nodiscard]]`)
     */
    [[nodiscard]] std::string generate_yaml_string() const;

    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, clock_frequency_min)                              // the minimum possible system clock frequency in MHz
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, clock_frequency_max)                              // the maximum possible system clock frequency in MHz
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, memory_clock_frequency_min)                       // the minimum possible memory clock frequency in MHz
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, memory_clock_frequency_max)                       // the maximum possible memory clock frequency in MHz
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, socket_clock_frequency_min)                       // the minimum possible socket clock frequency in MHz
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, socket_clock_frequency_max)                       // the maximum possible socket clock frequency in MHz
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::vector<double>, available_clock_frequencies)         // the available clock frequencies in MHz (slowest to fastest)
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::vector<double>, available_memory_clock_frequencies)  // the available memory clock frequencies in MHz (slowest to fastest)

    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, clock_frequency)                // the current system clock frequency in MHz
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, memory_clock_frequency)         // the current memory clock frequency in MHz
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, socket_clock_frequency)         // the current socket clock frequency in MHz
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(std::uint32_t, overdrive_level)         // the GPU overdrive percentage
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(std::uint32_t, memory_overdrive_level)  // the GPU memory overdrive percentage
};

/**
 * @brief Output the clock related @p samples to the given output-stream @p out.
 * @details In contrast to `rocm_smi_clock_samples::generate_yaml_string()`, outputs **all** clock related hardware samples, even if not supported by the current device (default initialized value).
 * @param[in,out] out the output-stream to write the clock related hardware samples to
 * @param[in] samples the ROCm SMI clock related samples
 * @return the output-stream
 */
std::ostream &operator<<(std::ostream &out, const rocm_smi_clock_samples &samples);

//*************************************************************************************************************************************//
//                                                            power samples                                                            //
//*************************************************************************************************************************************//

/**
 * @brief Wrapper class for all power related ROCm SMI hardware samples.
 */
class rocm_smi_power_samples {
    // befriend hardware sampler class
    friend class gpu_amd_hardware_sampler;

  public:
    /**
     * @brief Assemble the YAML string containing all available general hardware samples.
     * @details Hardware samples that are not supported by the current device are omitted in the YAML output.
     * @return the YAML string (`[[nodiscard]]`)
     */
    [[nodiscard]] std::string generate_yaml_string() const;

    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, power_management_limit)                      // the default power cap (W), may be different from power cap
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, power_enforced_limit)                        // if the GPU draws more power (W) than the power cap, the GPU may throttle
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::string, power_measurement_type)                 // the type of the power readings: either current power draw or average power draw
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::vector<std::string>, available_power_profiles)  // a list of the available power profiles

    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, power_usage)                     // the current GPU socket power draw in W
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, power_total_energy_consumption)  // the total power consumption since the last driver reload in J
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(std::string, power_profile)              // the current active power profile; one of 'available_power_profiles'
};

/**
 * @brief Output the power related @p samples to the given output-stream @p out.
 * @details In contrast to `rocm_smi_power_samples::generate_yaml_string()`, outputs **all** power related hardware samples, even if not supported by the current device (default initialized value).
 * @param[in,out] out the output-stream to write the power related hardware samples to
 * @param[in] samples the ROCm SMI power related samples
 * @return the output-stream
 */
std::ostream &operator<<(std::ostream &out, const rocm_smi_power_samples &samples);

//*************************************************************************************************************************************//
//                                                            memory samples                                                           //
//*************************************************************************************************************************************//

/**
 * @brief Wrapper class for all memory related ROCm SMI hardware samples.
 */
class rocm_smi_memory_samples {
    // befriend hardware sampler class
    friend class gpu_amd_hardware_sampler;

  public:
    /**
     * @brief Assemble the YAML string containing all available general hardware samples.
     * @details Hardware samples that are not supported by the current device are omitted in the YAML output.
     * @return the YAML string (`[[nodiscard]]`)
     */
    [[nodiscard]] std::string generate_yaml_string() const;

    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::uint64_t, memory_total)                 // the total available memory in Byte
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::uint64_t, visible_memory_total)         // the total visible available memory in Byte, may be smaller than the total memory
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::uint32_t, num_pcie_lanes_min)           // the minimum number of used PCIe lanes
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::uint32_t, num_pcie_lanes_max)           // the maximum number of used PCIe lanes
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::uint64_t, pcie_link_transfer_rate_min)  // the minimum PCIe link transfer rate in MT/s
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::uint64_t, pcie_link_transfer_rate_max)  // the maximum PCIe link transfer rate in MT/s

    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(std::uint64_t, memory_used)              // the currently used memory in Byte
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(std::uint64_t, memory_free)              // the currently free memory in Byte
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(std::uint32_t, num_pcie_lanes)           // the number of currently used PCIe lanes
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(std::uint64_t, pcie_link_transfer_rate)  // the current PCIe transfer rate in MT/s
};

/**
 * @brief Output the memory related @p samples to the given output-stream @p out.
 * @details In contrast to `rocm_smi_memory_samples::generate_yaml_string()`, outputs **all** memory related hardware samples, even if not supported by the current device (default initialized value).
 * @param[in,out] out the output-stream to write the memory related hardware samples to
 * @param[in] samples the ROCm SMI memory related samples
 * @return the output-stream
 */
std::ostream &operator<<(std::ostream &out, const rocm_smi_memory_samples &samples);

//*************************************************************************************************************************************//
//                                                         temperature samples                                                         //
//*************************************************************************************************************************************//

/**
 * @brief Wrapper class for all temperature related ROCm SMI hardware samples.
 */
class rocm_smi_temperature_samples {
    // befriend hardware sampler class
    friend class gpu_amd_hardware_sampler;

  public:
    /**
     * @brief Assemble the YAML string containing all available general hardware samples.
     * @details Hardware samples that are not supported by the current device are omitted in the YAML output.
     * @return the YAML string (`[[nodiscard]]`)
     */
    [[nodiscard]] std::string generate_yaml_string() const;

    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::uint32_t, num_fans)          // the number of fans (if any)
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(std::uint64_t, fan_speed_max)     // the maximum fan speed in RPM
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, temperature_min)          // the minimum temperature on the GPU's edge temperature sensor in °C
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, temperature_max)          // the maximum temperature on the GPU's edge temperature sensor in °C
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, memory_temperature_min)   // the minimum temperature on the GPU's memory temperature sensor in °C
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, memory_temperature_max)   // the maximum temperature on the GPU's memory temperature sensor in °C
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, hotspot_temperature_min)  // the minimum temperature on the GPU's hotspot temperature sensor in °C
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, hotspot_temperature_max)  // the maximum temperature on the GPU's hotspot temperature sensor in °C
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, hbm_0_temperature_min)    // the minimum temperature on the GPU's HBM0 temperature sensor in °C
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, hbm_0_temperature_max)    // the maximum temperature on the GPU's HBM0 temperature sensor in °C
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, hbm_1_temperature_min)    // the minimum temperature on the GPU's HBM1 temperature sensor in °C
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, hbm_1_temperature_max)    // the maximum temperature on the GPU's HBM1 temperature sensor in °C
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, hbm_2_temperature_min)    // the minimum temperature on the GPU's HBM2 temperature sensor in °C
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, hbm_2_temperature_max)    // the maximum temperature on the GPU's HBM2 temperature sensor in °C
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, hbm_3_temperature_min)    // the minimum temperature on the GPU's HBM3 temperature sensor in °C
    HWS_SAMPLE_STRUCT_FIXED_MEMBER(double, hbm_3_temperature_max)    // the maximum temperature on the GPU's HBM3 temperature sensor in °C

    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, fan_speed_percentage)  // the current fan speed in %
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, temperature)           // the current temperature on the GPU's edge temperature sensor in °C
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, hotspot_temperature)   // the current temperature on the GPU's hotspot temperature sensor in °C
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, memory_temperature)    // the current temperature on the GPU's memory temperature sensor in °C
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, hbm_0_temperature)     // the current temperature on the GPU's HBM0 temperature sensor in °C
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, hbm_1_temperature)     // the current temperature on the GPU's HBM1 temperature sensor in °C
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, hbm_2_temperature)     // the current temperature on the GPU's HBM2 temperature sensor in °C
    HWS_SAMPLE_STRUCT_SAMPLING_MEMBER(double, hbm_3_temperature)     // the current temperature on the GPU's HBM3 temperature sensor in °C
};

/**
 * @brief Output the temperature related @p samples to the given output-stream @p out.
 * @details In contrast to `rocm_smi_temperature_samples::generate_yaml_string()`, outputs **all** temperature related hardware samples, even if not supported by the current device (default initialized value).
 * @param[in,out] out the output-stream to write the temperature related hardware samples to
 * @param[in] samples the ROCm SMI temperature related samples
 * @return the output-stream
 */
std::ostream &operator<<(std::ostream &out, const rocm_smi_temperature_samples &samples);

}  // namespace hws

template <>
struct fmt::formatter<hws::rocm_smi_general_samples> : fmt::ostream_formatter { };

template <>
struct fmt::formatter<hws::rocm_smi_clock_samples> : fmt::ostream_formatter { };

template <>
struct fmt::formatter<hws::rocm_smi_power_samples> : fmt::ostream_formatter { };

template <>
struct fmt::formatter<hws::rocm_smi_memory_samples> : fmt::ostream_formatter { };

template <>
struct fmt::formatter<hws::rocm_smi_temperature_samples> : fmt::ostream_formatter { };

#endif  // HARDWARE_SAMPLING_GPU_AMD_ROCM_SMI_SAMPLES_HPP_
