/**
 * @file
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Defines a hardware sampler for Intel GPUs using Intel's Level Zero.
 */

#ifndef HARDWARE_SAMPLING_GPU_INTEL_HARDWARE_SAMPLER_HPP_
#define HARDWARE_SAMPLING_GPU_INTEL_HARDWARE_SAMPLER_HPP_
#pragma once

#include "hardware_sampling/gpu_intel/level_zero_device_handle.hpp"  // hws::detail::level_zero_device_handle
#include "hardware_sampling/gpu_intel/level_zero_samples.hpp"        // hws::{level_zero_general_samples, level_zero_clock_samples, level_zero_power_samples, level_zero_memory_samples, level_zero_temperature_samples}
#include "hardware_sampling/hardware_sampler.hpp"                    // hws::hardware_sampler
#include "hardware_sampling/sample_category.hpp"                     // hws::sample_category

#include "fmt/format.h"  // fmt::formatter, fmt::ostream_formatter

#include <atomic>   // std::atomic
#include <chrono>   // std::chrono::milliseconds, std::chrono_literals namespace
#include <cstddef>  // std::size_t
#include <iosfwd>   // std::ostream forward declaration
#include <string>   // std::string

namespace hws {

using namespace std::chrono_literals;

/**
 * @brief A hardware sampler for Intel GPUs.
 * @details Uses Intel's Level Zero library.
 */
class gpu_intel_hardware_sampler : public hardware_sampler {
  public:
    /**
     * @brief Construct a new Intel GPU hardware sampler for the default device with the default sampling interval.
     * @details If this is the first Intel GPU sampler, initializes the Level Zero environment.
     * @param[in] category the sample categories that are enabled for hardware sampling (default: all)
     */
    explicit gpu_intel_hardware_sampler(sample_category category = sample_category::all);
    /**
     * @brief Construct a new Intel GPU hardware sampler for device @p device_id with the default sampling interval.
     * @details If this is the first Intel GPU sampler, initializes the Level Zero environment.
     * @param[in] device_id the ID of the device to sample
     * @param[in] category the sample categories that are enabled for hardware sampling (default: all)
     */
    explicit gpu_intel_hardware_sampler(std::size_t device_id, sample_category category = sample_category::all);
    /**
     * @brief Construct a new Intel GPU hardware sampler for the default device with the @p sampling_interval.
     * @details If this is the first Intel GPU sampler, initializes the Level Zero environment.
     * @param[in] sampling_interval the used sampling interval
     * @param[in] category the sample categories that are enabled for hardware sampling (default: all)
     */
    explicit gpu_intel_hardware_sampler(std::chrono::milliseconds sampling_interval, sample_category category = sample_category::all);
    /**
     * @brief Construct a new Intel GPU hardware sampler for device @p device_id with the @p sampling_interval.
     * @details If this is the first Intel GPU sampler, initializes the Level Zero environment.
     * @param[in] device_id the ID of the device to sample
     * @param[in] sampling_interval the used sampling interval
     * @param[in] category the sample categories that are enabled for hardware sampling (default: all)
     */
    gpu_intel_hardware_sampler(std::size_t device_id, std::chrono::milliseconds sampling_interval, sample_category category = sample_category::all);

    /**
     * @brief Delete the copy-constructor (already implicitly deleted due to the base class's std::atomic member).
     */
    gpu_intel_hardware_sampler(const gpu_intel_hardware_sampler &) = delete;
    /**
     * @brief Delete the move-constructor (already implicitly deleted due to the base class's std::atomic member).
     */
    gpu_intel_hardware_sampler(gpu_intel_hardware_sampler &&) noexcept = delete;
    /**
     * @brief Delete the copy-assignment operator (already implicitly deleted due to the base class's std::atomic member).
     */
    gpu_intel_hardware_sampler &operator=(const gpu_intel_hardware_sampler &) = delete;
    /**
     * @brief Delete the move-assignment operator (already implicitly deleted due to the base class's std::atomic member).
     */
    gpu_intel_hardware_sampler &operator=(gpu_intel_hardware_sampler &&) noexcept = delete;

    /**
     * @brief Destruct the Intel GPU hardware sampler. If the sampler is still running, stops it.
     */
    ~gpu_intel_hardware_sampler() override;

    /**
     * @brief Return the general Intel GPU samples of this hardware sampler.
     * @return the general Intel GPU samples (`[[nodiscard]]`)
     */
    [[nodiscard]] const level_zero_general_samples &general_samples() const noexcept { return general_samples_; }

    /**
     * @brief Return the clock related Intel GPU samples of this hardware sampler.
     * @return the clock related Intel GPU samples (`[[nodiscard]]`)
     */
    [[nodiscard]] const level_zero_clock_samples &clock_samples() const noexcept { return clock_samples_; }

    /**
     * @brief Return the power related Intel GPU samples of this hardware sampler.
     * @return the power related Intel GPU samples (`[[nodiscard]]`)
     */
    [[nodiscard]] const level_zero_power_samples &power_samples() const noexcept { return power_samples_; }

    /**
     * @brief Return the memory related Intel GPU samples of this hardware sampler.
     * @return the memory related Intel GPU samples (`[[nodiscard]]`)
     */
    [[nodiscard]] const level_zero_memory_samples &memory_samples() const noexcept { return memory_samples_; }

    /**
     * @brief Return the temperature related Intel GPU samples of this hardware sampler.
     * @return the temperature related Intel GPU samples (`[[nodiscard]]`)
     */
    [[nodiscard]] const level_zero_temperature_samples &temperature_samples() const noexcept { return temperature_samples_; }

    /**
     * @copydoc hws::hardware_sampler::device_identification
     */
    std::string device_identification() const final;

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
    detail::level_zero_device_handle device_;

    /// The general Intel GPU samples.
    level_zero_general_samples general_samples_{};
    /// The clock related Intel GPU samples.
    level_zero_clock_samples clock_samples_{};
    /// The power related Intel GPU samples.
    level_zero_power_samples power_samples_{};
    /// The memory related Intel GPU samples.
    level_zero_memory_samples memory_samples_{};
    /// The temperature related Intel GPU samples.
    level_zero_temperature_samples temperature_samples_{};

    /// The total number of currently active Intel GPU hardware samplers.
    inline static std::atomic<int> instances_{ 0 };
    /// True if the Level Zero environment has been successfully initialized (only done by a single hardware sampler).
    inline static std::atomic<bool> init_finished_{ false };
};

/**
 * @brief Output all Intel GPU samples gathered by the @p sampler to the given output-stream @p out.
 * @details Sets `std::ios_base::failbit` if the @p sampler is still sampling.
 * @param[in,out] out the output-stream to write the Intel GPU samples to
 * @param[in] sampler the Intel GPU hardware sampler
 * @return the output-stream
 */
std::ostream &operator<<(std::ostream &out, const gpu_intel_hardware_sampler &sampler);

}  // namespace hws

/// @cond Doxygen_suppress

template <>
struct fmt::formatter<hws::gpu_intel_hardware_sampler> : fmt::ostream_formatter { };

/// @endcond

#endif  // HARDWARE_SAMPLING_GPU_INTEL_HARDWARE_SAMPLER_HPP_
