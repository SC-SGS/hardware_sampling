/**
 * @file
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Defines the base class for all hardware samplers.
 */

#ifndef HWS_HARDWARE_SAMPLER_HPP_
#define HWS_HARDWARE_SAMPLER_HPP_
#pragma once

#include "hws/event.hpp"            // hws::event
#include "hws/sample_category.hpp"  // hws::sample_category

#include <atomic>      // std::atomic
#include <chrono>      // std::chrono::{system_clock::time_point, steady_clock::time_point, milliseconds}
#include <cstddef>     // std::size_t
#include <filesystem>  // std::filesystem::path
#include <string>      // std::string
#include <thread>      // std::thread
#include <vector>      // std::vector

namespace hws {

/**
 * @brief The base class for all specialized hardware samplers.
 */
class hardware_sampler {
  public:
    /**
     * @brief Construct a new hardware sampler with the provided @p sampling_interval.
     * @param[in] sampling_interval the used sampling interval
     * @param[in] category the sample categories that are enabled for hardware sampling
     * @throws std::invalid_argument if the @p sampling_interval is zero
     */
    hardware_sampler(std::chrono::milliseconds sampling_interval, sample_category category);

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
     * @brief Check whether this hardware sampler is currently sampling.
     * @return `true` if the hardware sampler is currently sampling, `false` otherwise (`[[nodiscard]]`)
     */
    [[nodiscard]] bool is_sampling() const noexcept;
    /**
     * @brief Check whether this hardware sampler has already stopped sampling.
     * @return `true` if the hardware sampler has already stopped sampling, `false` otherwise (`[[nodiscard]]`)
     */
    [[nodiscard]] bool has_sampling_stopped() const noexcept;

    /**
     * @brief Add a new event.
     * @param e the event
     */
    void add_event(event e);
    /**
     * @brief Add a new event.
     * @param[in] time_point the time point when the event occurred
     * @param[in] name the name of the event
     */
    void add_event(decltype(event::time_point) time_point, decltype(event::name) name);
    /**
     * @brief Add a new event. The time_point will be the current time.
     * @param[in] name the name of the event
     */
    void add_event(decltype(event::name) name);

    /**
     * @brief Return the number of recorded events.
     * @return the number of events (`[[nodiscard]]`)
     */
    [[nodiscard]] std::size_t num_events() const noexcept { return events_.size(); }

    /**
     * @brief Return a vector of all recorded events.
     * @return the events (`[[nodiscard]]`)
     */
    [[nodiscard]] const std::vector<event> &get_events() const noexcept { return events_; }

    /**
     * @brief Return the event at index @p idx.
     * @param[in] idx the event to return
     * @throws std::out_of_range the the @p idx is out of bounce
     * @return the event at index @p idx (`[[nodiscard]]`)
     */
    [[nodiscard]] event get_event(std::size_t idx) const;

    /**
     * @brief Return the time points the samples of this hardware sampler occurred.
     * @return the time points (`[[nodiscard]]`)
     */
    [[nodiscard]] std::vector<std::chrono::steady_clock::time_point> sampling_time_points() const noexcept { return time_points_; }

    /**
     * @brief Return the sampling interval of this hardware sampler.
     * @return the samping interval in milliseconds (`[[nodiscard]]`)
     */
    [[nodiscard]] std::chrono::milliseconds sampling_interval() const noexcept { return sampling_interval_; }

    /**
     * @brief Dump the hardware samples to the YAML file with @p filename.
     * @param[in] filename the YAML file to append the hardware samples to
     */
    void dump_yaml(const char *filename) const;
    /**
     * @copydoc hws::hardware_sampler::dump_yaml(const char *) const
     */
    void dump_yaml(const std::string &filename) const;
    /**
     * @copydoc hws::hardware_sampler::dump_yaml(const char *) const
     */
    void dump_yaml(const std::filesystem::path &filename) const;

    /**
     * @brief Return the unique device identification. Can be used as unique key in the YAML string.
     * @return the unique device identification (`[[nodiscard]]`)
     */
    [[nodiscard]] virtual std::string device_identification() const = 0;

    /**
     * @brief Return the hardware samples as well as events and time points as YAML string.
     * @return the YAML content as string (`[[nodiscard]]`)
     */
    [[nodiscard]] std::string as_yaml_string() const;
    /**
     * @brief Return only the hardware samples as YAML string.
     * @throws std::runtime_error if sampling is still running
     * @return the YAML content as string (`[[nodiscard]]`)
     */
    [[nodiscard]] virtual std::string samples_only_as_yaml_string() const = 0;

  protected:
    /**
     * @brief Getter the hardware samples. Called in another std::thread.
     */
    virtual void sampling_loop() = 0;

    /**
     * @brief Add a new time point to this hardware sampler. Called during the sampling loop.
     * @param time_point the new time point to add
     */
    void add_time_point(std::chrono::steady_clock::time_point time_point);

    /**
     * @brief Check whether the @p category is currently enabled for hardware sampling or not.
     * @param[in] category the sample_category to check
     * @return Returns `true` if @p category is enabled for sampling, otherwise `false` (`[[nodiscard]]`)
     */
    [[nodiscard]] bool sample_category_enabled(sample_category category) const noexcept;

  private:
    /// A boolean flag indicating whether the sampling has already started.
    std::atomic<bool> sampling_started_{ false };
    /// A boolean flag indicating whether the sampling has already stopped.
    std::atomic<bool> sampling_stopped_{ false };
    /// A boolean flag indicating whether the sampling is currently running.
    std::atomic<bool> sampling_running_{ false };

    /// The wallclock time where the hardware sampling started.
    std::chrono::system_clock::time_point start_date_time_{};

    /// The different tracked events.
    std::vector<event> events_{};

    /// The std::thread used to getter the hardware samples.
    std::thread sampling_thread_{};

    /// The time points at which this hardware sampler sampled its values.
    std::vector<std::chrono::steady_clock::time_point> time_points_{};

    /// The sampling interval of this hardware sampler.
    const std::chrono::milliseconds sampling_interval_{};

    /// The bitmask of sample categories to use.
    const sample_category sample_category_{};
};

}  // namespace hws

#endif  // HWS_HARDWARE_SAMPLER_HPP_
