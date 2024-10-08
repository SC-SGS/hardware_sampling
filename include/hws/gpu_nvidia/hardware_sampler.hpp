/**
 * @file
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Defines a hardware sampler for NVIDIA GPUs using NVIDIA's Management Library (NVML).
 */

#ifndef HWS_GPU_NVIDIA_HARDWARE_SAMPLER_HPP_
#define HWS_GPU_NVIDIA_HARDWARE_SAMPLER_HPP_
#pragma once

#include "hws/gpu_nvidia/nvml_device_handle.hpp"  // hws::nvml_device_handle
#include "hws/gpu_nvidia/nvml_samples.hpp"        // hws::{nvml_general_samples, nvml_clock_samples, nvml_power_samples, nvml_memory_samples, nvml_temperature_samples}
#include "hws/hardware_sampler.hpp"               // hws::hardware_sampler
#include "hws/sample_category.hpp"                // hws::sample_category

#include "fmt/format.h"  // fmt::formatter, fmt::ostream_formatter

#include <atomic>   // std::atomic
#include <chrono>   // std::chrono::milliseconds, std::chrono_literals namespace
#include <cstddef>  // std::size_t
#include <iosfwd>   // std::ostream forward declaration
#include <string>   // std::string

namespace hws {

using namespace std::chrono_literals;

/**
 * @brief A hardware sampler for NVIDIA GPUs.
 * @details Uses NVIDIA's NVML library.
 */
class gpu_nvidia_hardware_sampler : public hardware_sampler {
  public:
    /**
     * @brief Construct a new NVIDIA GPU hardware sampler for the default device with the default sampling interval.
     * @details If this is the first NVIDIA GPU sampler, initializes the NVML environment.
     * @param[in] category the sample categories that are enabled for hardware sampling (default: all)
     */
    explicit gpu_nvidia_hardware_sampler(sample_category category = sample_category::all);
    /**
     * @brief Construct a new NVIDIA GPU hardware sampler for device @p device_id with the default sampling interval.
     * @details If this is the first NVIDIA GPU sampler, initializes the NVML environment.
     * @param[in] device_id the ID of the device to sample
     * @param[in] category the sample categories that are enabled for hardware sampling (default: all)
     */
    explicit gpu_nvidia_hardware_sampler(std::size_t device_id, sample_category category = sample_category::all);
    /**
     * @brief Construct a new NVIDIA GPU hardware sampler for the default device with the @p sampling_interval.
     * @details If this is the first NVIDIA GPU sampler, initializes the NVML environment.
     * @param[in] sampling_interval the used sampling interval
     * @param[in] category the sample categories that are enabled for hardware sampling (default: all)
     */
    explicit gpu_nvidia_hardware_sampler(std::chrono::milliseconds sampling_interval, sample_category category = sample_category::all);
    /**
     * @brief Construct a new NVIDIA GPU hardware sampler for device @p device_id with the @p sampling_interval.
     * @details If this is the first NVIDIA GPU sampler, initializes the NVML environment.
     * @param[in] device_id the ID of the device to sample
     * @param[in] sampling_interval the used sampling interval
     * @param[in] category the sample categories that are enabled for hardware sampling (default: all)
     */
    gpu_nvidia_hardware_sampler(std::size_t device_id, std::chrono::milliseconds sampling_interval, sample_category category = sample_category::all);

    /**
     * @brief Delete the copy-constructor (already implicitly deleted due to the base class's std::atomic member).
     */
    gpu_nvidia_hardware_sampler(const gpu_nvidia_hardware_sampler &) = delete;
    /**
     * @brief Delete the move-constructor (already implicitly deleted due to the base class's std::atomic member).
     */
    gpu_nvidia_hardware_sampler(gpu_nvidia_hardware_sampler &&) noexcept = delete;
    /**
     * @brief Delete the copy-assignment operator (already implicitly deleted due to the base class's std::atomic member).
     */
    gpu_nvidia_hardware_sampler &operator=(const gpu_nvidia_hardware_sampler &) = delete;
    /**
     * @brief Delete the move-assignment operator (already implicitly deleted due to the base class's std::atomic member).
     */
    gpu_nvidia_hardware_sampler &operator=(gpu_nvidia_hardware_sampler &&) noexcept = delete;

    /**
     * @brief Destruct the NVIDIA GPU hardware sampler. If the sampler is still running, stops it.
     * @details If this is the last NVIDIA GPU sampler, cleans up the NVML environment.
     */
    ~gpu_nvidia_hardware_sampler() override;

    /**
     * @brief Return the general NVIDIA GPU samples of this hardware sampler.
     * @return the general NVIDIA GPU samples (`[[nodiscard]]`)
     */
    [[nodiscard]] const nvml_general_samples &general_samples() const noexcept { return general_samples_; }

    /**
     * @brief Return the clock related NVIDIA GPU samples of this hardware sampler.
     * @return the clock related NVIDIA GPU samples (`[[nodiscard]]`)
     */
    [[nodiscard]] const nvml_clock_samples &clock_samples() const noexcept { return clock_samples_; }

    /**
     * @brief Return the power related NVIDIA GPU samples of this hardware sampler.
     * @return the power related NVIDIA GPU samples (`[[nodiscard]]`)
     */
    [[nodiscard]] const nvml_power_samples &power_samples() const noexcept { return power_samples_; }

    /**
     * @brief Return the memory related NVIDIA GPU samples of this hardware sampler.
     * @return the memory related NVIDIA GPU samples (`[[nodiscard]]`)
     */
    [[nodiscard]] const nvml_memory_samples &memory_samples() const noexcept { return memory_samples_; }

    /**
     * @brief Return the temperature related NVIDIA GPU samples of this hardware sampler.
     * @return the temperature related NVIDIA GPU samples (`[[nodiscard]]`)
     */
    [[nodiscard]] const nvml_temperature_samples &temperature_samples() const noexcept { return temperature_samples_; }

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

    /// The device handle for the device to sample.
    detail::nvml_device_handle device_{};

    /// The general NVIDIA GPU samples.
    nvml_general_samples general_samples_{};
    /// The clock related NVIDIA GPU samples.
    nvml_clock_samples clock_samples_{};
    /// The power related NVIDIA GPU samples.
    nvml_power_samples power_samples_{};
    /// The memory related NVIDIA GPU samples.
    nvml_memory_samples memory_samples_{};
    /// The temperature related NVIDIA GPU samples.
    nvml_temperature_samples temperature_samples_{};

    /// The total number of currently active NVIDIA GPU hardware samplers.
    inline static std::atomic<int> instances_{ 0 };
    /// True if the NVML environment has been successfully initialized (only done by a single hardware sampler).
    inline static std::atomic<bool> init_finished_{ false };
};

/**
 * @brief Output all NVIDIA GPU samples gathered by the @p sampler to the given output-stream @p out.
 * @details Sets `std::ios_base::failbit` if the @p sampler is still sampling.
 * @param[in,out] out the output-stream to write the NVIDIA GPU samples to
 * @param[in] sampler the NVIDIA GPU hardware sampler
 * @return the output-stream
 */
std::ostream &operator<<(std::ostream &out, const gpu_nvidia_hardware_sampler &sampler);

}  // namespace hws

/// @cond Doxygen_suppress

template <>
struct fmt::formatter<hws::gpu_nvidia_hardware_sampler> : fmt::ostream_formatter { };

/// @endcond

#endif  // HWS_GPU_NVIDIA_HARDWARE_SAMPLER_HPP_
