/**
 * @file
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Defines the base class for all hardware samplers.
 */

#ifndef HARDWARE_SAMPLING_HARDWARE_SAMPLER_HPP_
#define HARDWARE_SAMPLING_HARDWARE_SAMPLER_HPP_
#pragma once

#include <atomic>  // std::atomic
#include <chrono>  // std::chrono::{steady_clock::time_point, milliseconds}
#include <string>  // std::string
#include <thread>  // std::thread
#include <vector>  // std::vector

namespace hws {

/**
 * @brief The base class for all specialized hardware samplers.
 */
class hardware_sampler {
  public:
    /**
     * @brief Construct a new hardware sampler with the provided @p sampling_interval.
     * @param[in] sampling_interval the used sampling interval
     */
    explicit hardware_sampler(std::chrono::milliseconds sampling_interval);

    /**
     * @brief Delete the copy-constructor (already implicitly deleted due to the std::atomic member).
     */
    hardware_sampler(const hardware_sampler &) = delete;
    /**
     * @brief Delete the move-constructor (already implicitly deleted due to the std::atomic member).
     */
    hardware_sampler(hardware_sampler &&) noexcept = delete;
    /**
     * @brief Delete the copy-assignment operator (already implicitly deleted due to the std::atomic member).
     */
    hardware_sampler &operator=(const hardware_sampler &) = delete;
    /**
     * @brief Delete the move-assignment operator (already implicitly deleted due to the std::atomic member).
     */
    hardware_sampler &operator=(hardware_sampler &&) noexcept = delete;

    /**
     * @brief Pure virtual default destructor.
     */
    virtual ~hardware_sampler() = 0;

    /**
     * @brief Start hardware sampling in a new std::thread.
     * @details Once a hardware sampler has been started, it can never be started again, even if `hardware_sampler::stop_sampling` has been called.
     * @throws std::runtime_error if the hardware sampler has already been started
     */
    void start_sampling();
    /**
     * @brief Stop hardware sampling. Signals the running std::thread to stop sampling and joins it.
     * @details Once a hardware sampler has been stopped, it can never be stopped again.
     * @throws std::runtime_error if the hardware sampler hasn't been started yet
     * @throws std::runtime_error if the hardware sampler has already been stopped
     */
    void stop_sampling();
    /**
     * @brief Pause hardware sampling.
     */
    void pause_sampling();
    /**
     * @brief Resume hardware sampling.
     * @throws std::runtime_error if the hardware sampler has already been stopped
     */
    void resume_sampling();

    /**
     * @brief Check whether this hardware sampler has already started sampling.
     * @return `true` if the hardware sampler has already started sampling, `false` otherwise (`[[nodiscard]]`)
     */
    [[nodiscard]] bool has_sampling_started() const noexcept;
    /**
     * @brief Check whether this hardware sampler has currently sampling.
     * @return `true` if the hardware sampler is currently sampling, `false` otherwise (`[[nodiscard]]`)
     */
    [[nodiscard]] bool is_sampling() const noexcept;
    /**
     * @brief Check whether this hardware sampler has already stopped sampling.
     * @return `true` if the hardware sampler has already stopped sampling, `false` otherwise (`[[nodiscard]]`)
     */
    [[nodiscard]] bool has_sampling_stopped() const noexcept;

    /**
     * @brief Return the time points the samples of this hardware sampler occurred.
     * @return the time points (`[[nodiscard]]`)
     */
    [[nodiscard]] std::vector<std::chrono::steady_clock::time_point> time_points() const noexcept { return time_points_; }

    /**
     * @brief Return the sampling interval of this hardware sampler.
     * @return the samping interval in milliseconds (`[[nodiscard]]`)
     */
    [[nodiscard]] std::chrono::milliseconds sampling_interval() const noexcept { return sampling_interval_; }

  protected:
    /**
     * @brief Getter the hardware samples. Called in another std::thread.
     */
    virtual void sampling_loop() = 0;

    /**
     * @brief Add a new time point to this hardware sampler. Called during the sampling loop.
     * @param time_point the new time point to add
     */
    void add_time_point(const std::chrono::steady_clock::time_point time_point) { time_points_.push_back(time_point); }

  private:
    /// A boolean flag indicating whether the sampling has already started.
    std::atomic<bool> sampling_started_{ false };
    /// A boolean flag indicating whether the sampling has already stopped.
    std::atomic<bool> sampling_stopped_{ false };
    /// A boolean flag indicating whether the sampling is currently running.
    std::atomic<bool> sampling_running_{ false };

    /// The std::thread used to getter the hardware samples.
    std::thread sampling_thread_{};

    /// The time points at which this hardware sampler sampled its values.
    std::vector<std::chrono::steady_clock::time_point> time_points_{};

    /// The sampling interval of this hardware sampler.
    const std::chrono::milliseconds sampling_interval_{};
};

}  // namespace hws

#endif  // HARDWARE_SAMPLING_HARDWARE_SAMPLER_HPP_
