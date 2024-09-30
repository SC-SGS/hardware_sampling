/**
 * @file
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Defines a hardware sampler for the whole system, i.e., automatically creates CPU and GPU hardware samples if the respective sampler and hardware are available.
 */

#ifndef HARDWARE_SAMPLING_SYSTEM_HARDWARE_SAMPLER_HPP_
#define HARDWARE_SAMPLING_SYSTEM_HARDWARE_SAMPLER_HPP_

#include "hardware_sampling/event.hpp"             // hws::event
#include "hardware_sampling/hardware_sampler.hpp"  // hws::hardware_sampler
#include "hardware_sampling/sample_category.hpp"   // hws::sample_category

#include <chrono>      // std::chrono::{milliseconds, steady_clock::time_point}
#include <cstddef>     // std::size_t
#include <filesystem>  // std::filesystem::path
#include <memory>      // std::unique_ptr
#include <string>      // std::string
#include <vector>      // std::vector

namespace hws {

/**
 * @brief A hardware sampler for the whole system.
 * @details Enables hardware samplers for which hardware is available and the CMake configuration found the respective dependencies.
 */
class system_hardware_sampler {
  public:
    /**
     * @brief Construct hardware samplers with the default sampling interval.
     * @param[in] category the sample categories that are enabled for hardware sampling (default: all)
     */
    explicit system_hardware_sampler(sample_category category = sample_category::all);
    /**
     * @brief Construct hardware samplers with the provided @p sampling_interval.
     * @param[in] sampling_interval the used sampling interval
     * @param[in] category the sample categories that are enabled for hardware sampling (default: all)
     */
    explicit system_hardware_sampler(std::chrono::milliseconds sampling_interval, sample_category category = sample_category::all);

    /**
     * @brief Delete the copy-constructor.
     */
    system_hardware_sampler(const system_hardware_sampler &) = delete;
    /**
     * @brief Delete the move-constructor.
     */
    system_hardware_sampler(system_hardware_sampler &&) noexcept = delete;
    /**
     * @brief Delete the copy-assignment operator.
     */
    system_hardware_sampler &operator=(const system_hardware_sampler &) = delete;
    /**
     * @brief Delete the move-assignment operator.
     */
    system_hardware_sampler &operator=(system_hardware_sampler &&) noexcept = delete;

    /**
     * @brief Explicitly use the default destructor.
     */
    ~system_hardware_sampler() = default;

    /**
     * @brief Start hardware sampling for all wrapped hardware samplers.
     */
    void start_sampling();
    /**
     * @brief Stop hardware sampling for all wrapped hardware samplers.
     */
    void stop_sampling();
    /**
     * @brief Pause hardware sampling for all wrapped hardware samplers.
     */
    void pause_sampling();
    /**
     * @brief Resume hardware sampling for all wrapped hardware samplers.
     */
    void resume_sampling();

    /**
     * @brief Check whether the hardware samplers have already started sampling.
     * @return `true` if **all** hardware samplers have already started sampling, `false` otherwise (`[[nodiscard]]`)
     */
    [[nodiscard]] bool has_sampling_started() const noexcept;
    /**
     * @brief Check whether the hardware samplers are currently sampling.
     * @return `true` if **all** hardware samplers are currently sampling, `false` otherwise (`[[nodiscard]]`)
     */
    [[nodiscard]] bool is_sampling() const noexcept;
    /**
     * @brief Check whether the hardware samplers have already stopped sampling.
     * @return `true` if **all** hardware samplers have already stopped sampling, `false` otherwise (`[[nodiscard]]`)
     */
    [[nodiscard]] bool has_sampling_stopped() const noexcept;

    /**
     * @brief Add a new event to all hardware samplers.
     * @param e the event
     */
    void add_event(event e);
    /**
     * @brief Add a new event to all hardware samplers.
     * @param[in] time_point the time point when the event occurred
     * @param[in] name the name of the event
     */
    void add_event(decltype(event::time_point) time_point, decltype(event::name) name);
    /**
     * @brief Add a new event to all hardware samplers. The time_point will be the current time.
     * @param[in] name the name of the event
     */
    void add_event(decltype(event::name) name);

    /**
     * @brief Return the number of recorded events separately for each hardware sampler.
     * @return the number of events per hardware sampler (`[[nodiscard]]`)
     */
    [[nodiscard]] std::vector<std::size_t> num_events() const;
    /**
     * @brief Return the number of recorded events separately for each hardware sampler.
     * @return the events per hardware sampler (`[[nodiscard]]`)
     */
    [[nodiscard]] std::vector<std::vector<event>> get_events() const;
    /**
     * @brief Return the time points the samples separately for each hardware sampler.
     * @return the time points per hardware sampler (`[[nodiscard]]`)
     */
    [[nodiscard]] std::vector<std::vector<std::chrono::steady_clock::time_point>> sampling_time_points() const;
    /**
     * @brief Return the sampling interval separately for each hardware sampler.
     * @return the samping interval in milliseconds per hardware sampler (`[[nodiscard]]`)
     */
    [[nodiscard]] std::vector<std::chrono::milliseconds> sampling_interval() const;

    /**
     * @brief The number of hardware samplers available for the whole system.
     * @return the number of hardware samplers (`[[nodiscard]]`)
     */
    [[nodiscard]] std::size_t num_samplers() const noexcept;
    /**
     * @brief The hardware samplers available for the whole system.
     * @return all available hardware samplers (`[[nodiscard]]`)
     */
    [[nodiscard]] const std::vector<std::unique_ptr<hardware_sampler>> &samplers() const noexcept;
    /**
     * @copydoc hws::system_hardware_sampler::samplers() const
     */
    [[nodiscard]] std::vector<std::unique_ptr<hardware_sampler>> &samplers() noexcept;
    /**
     * @brief Return the hardware sampler at index @p idx.
     * @param[in] idx the index of the hardware sampler
     * @throws std::out_of_range if @p idx is out-of-range
     * @return the hardware sampler at index @p idx (`[[nodiscard]]`)
     */
    [[nodiscard]] const std::unique_ptr<hardware_sampler> &sampler(std::size_t idx) const;
    /**
     * @copydoc hws::system_hardware_sampler::sampler(std::size_t idx) const
     */
    [[nodiscard]] std::unique_ptr<hardware_sampler> &sampler(std::size_t idx);

    /**
     * @brief Dump the hardware samples of all hardware samplers to the YAML file with @p filename.
     * @param[in] filename the YAML file to append the hardware samples to
     */
    void dump_yaml(const char *filename) const;
    /**
     * @copydoc hws::system_hardware_sampler::dump_yaml(const char *) const
     */
    void dump_yaml(const std::string &filename) const;
    /**
     * @copydoc hws::system_hardware_sampler::dump_yaml(const char *) const
     */
    void dump_yaml(const std::filesystem::path &filename) const;

    /**
     * @brief Return the hardware samples as YAML string.
     * @return the YAML content as string (`[[nodiscard]]`)
     */
    [[nodiscard]] std::string as_yaml_string() const;
    /**
     * @brief Return only the hardware samples as YAML string.
     * @throws std::runtime_error if sampling is still running
     * @return the YAML content as string (`[[nodiscard]]`)
     */
    [[nodiscard]] std::string samples_only_as_yaml_string() const;

  private:
    /// The different hardware sampler for the current system.
    std::vector<std::unique_ptr<hardware_sampler>> samplers_;
};

}  // namespace hws

#endif  // HARDWARE_SAMPLING_SYSTEM_HARDWARE_SAMPLER_HPP_
