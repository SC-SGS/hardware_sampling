/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hws/gpu_amd/hardware_sampler.hpp"

#include "hws/gpu_amd/rocm_smi_samples.hpp"  // hws::{rocm_smi_general_samples, rocm_smi_clock_samples, rocm_smi_power_samples, rocm_smi_memory_samples, rocm_smi_temperature_samples}
#include "hws/gpu_amd/utility.hpp"           // hws::detail::performance_level_to_string, HWS_ROCM_SMI_ERROR_CHECK
#include "hws/hardware_sampler.hpp"          // hws::hardware_sampler
#include "hws/sample_category.hpp"           // hws::sample_category
#include "hws/utility.hpp"                   // hws::detail::time_points_to_epoch

#include "fmt/chrono.h"           // direct formatting of std::chrono types
#include "fmt/format.h"           // fmt::format
#include "fmt/ranges.h"           // fmt::join
#include "hip/hip_runtime_api.h"  // HIP runtime functions
#include "rocm_smi/rocm_smi.h"    // ROCm SMI runtime functions

#include <chrono>     // std::chrono::{steady_clock, duration_cast, milliseconds}
#include <cstddef>    // std::size_t
#include <cstdint>    // std::uint32_t, std::uint64_t
#include <exception>  // std::exception, std::terminate
#include <ios>        // std::ios_base
#include <iostream>   // std::cerr, std::endl
#include <optional>   // std::optional
#include <ostream>    // std::ostream
#include <stdexcept>  // std::runtime_error
#include <string>     // std::string
#include <thread>     // std::this_thread
#include <utility>    // std::move
#include <vector>     // std::vector

namespace hws {

gpu_amd_hardware_sampler::gpu_amd_hardware_sampler(const sample_category category) :
    gpu_amd_hardware_sampler{ 0, HWS_SAMPLING_INTERVAL, category } { }

gpu_amd_hardware_sampler::gpu_amd_hardware_sampler(const std::size_t device_id, const sample_category category) :
    gpu_amd_hardware_sampler{ device_id, HWS_SAMPLING_INTERVAL, category } { }

gpu_amd_hardware_sampler::gpu_amd_hardware_sampler(const std::chrono::milliseconds sampling_interval, const sample_category category) :
    gpu_amd_hardware_sampler{ 0, sampling_interval, category } { }

gpu_amd_hardware_sampler::gpu_amd_hardware_sampler(const std::size_t device_id, const std::chrono::milliseconds sampling_interval, const sample_category category) :
    hardware_sampler{ sampling_interval, category },
    device_id_{ static_cast<std::uint32_t>(device_id) } {
    // make sure that rsmi_init is only called once for all instances
    if (instances_++ == 0) {
        HWS_ROCM_SMI_ERROR_CHECK(rsmi_init(std::uint64_t{ 0 }))
        // notify that initialization has been finished
        init_finished_ = true;
    } else {
        // wait until init has been finished!
        while (!init_finished_) { }
    }
}

gpu_amd_hardware_sampler::~gpu_amd_hardware_sampler() {
    try {
        // if this hardware sampler is still sampling, stop it
        if (this->has_sampling_started() && !this->has_sampling_stopped()) {
            this->stop_sampling();
        }

        // the last instance must shut down the ROCm SMI runtime
        // make sure that rsmi_shut_down is only called once
        if (--instances_ == 0) {
            HWS_ROCM_SMI_ERROR_CHECK(rsmi_shut_down())
            // reset init_finished flag
            init_finished_ = false;
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::terminate();
    }
}

void gpu_amd_hardware_sampler::sampling_loop() {
    //
    // add samples where we only have to retrieve the value once
    //

    this->add_time_point(std::chrono::steady_clock::now());

    double initial_total_power_consumption{};  // initial total power consumption in J

    // retrieve initial general information
    if (this->sample_category_enabled(sample_category::general)) {
        // fixed information -> only retrieved once
        // the byte order is given by AMD directly
        general_samples_.byte_order_ = "Little Endian";

        hipDeviceProp_t prop{};
        if (hipGetDeviceProperties(&prop, static_cast<int>(device_id_)) == hipSuccess) {
            const std::string architecture{ prop.gcnArchName };
            general_samples_.architecture_ = architecture.substr(0, architecture.find_first_of('\0'));
        }

        std::string vendor_id(static_cast<std::string::size_type>(1024), '\0');
        if (rsmi_dev_vendor_name_get(device_id_, vendor_id.data(), vendor_id.size()) == RSMI_STATUS_SUCCESS) {
            general_samples_.vendor_id_ = vendor_id.substr(0, vendor_id.find_first_of('\0'));
        }

        std::string name(static_cast<std::string::size_type>(1024), '\0');
        if (rsmi_dev_name_get(device_id_, name.data(), name.size()) == RSMI_STATUS_SUCCESS) {
            general_samples_.name_ = name.substr(0, name.find_first_of('\0'));
        }

        // queried samples -> retrieved every iteration if available
        rsmi_dev_perf_level_t pstate{};
        if (rsmi_dev_perf_level_get(device_id_, &pstate) == RSMI_STATUS_SUCCESS) {
            general_samples_.performance_level_ = decltype(general_samples_.performance_level_)::value_type{ detail::performance_level_to_string(pstate) };
        }

        decltype(general_samples_.compute_utilization_)::value_type::value_type utilization_gpu{};
        if (rsmi_dev_busy_percent_get(device_id_, &utilization_gpu) == RSMI_STATUS_SUCCESS) {
            general_samples_.compute_utilization_ = decltype(general_samples_.compute_utilization_)::value_type{ utilization_gpu };
        }

        decltype(general_samples_.memory_utilization_)::value_type::value_type utilization_mem{};
        if (rsmi_dev_memory_busy_percent_get(device_id_, &utilization_mem) == RSMI_STATUS_SUCCESS) {
            general_samples_.memory_utilization_ = decltype(general_samples_.memory_utilization_)::value_type{ utilization_mem };
        }
    }

    // retrieve initial clock related information
    if (this->sample_category_enabled(sample_category::clock)) {
        rsmi_frequencies_t frequency_info{};
        if (rsmi_dev_gpu_clk_freq_get(device_id_, RSMI_CLK_TYPE_SYS, &frequency_info) == RSMI_STATUS_SUCCESS) {
            clock_samples_.clock_frequency_min_ = static_cast<decltype(clock_samples_.clock_frequency_min_)::value_type>(frequency_info.frequency[0]) / 1000'000.0;
            clock_samples_.clock_frequency_max_ = static_cast<decltype(clock_samples_.clock_frequency_max_)::value_type>(frequency_info.frequency[frequency_info.num_supported - 1]) / 1000'000.0;
            decltype(clock_samples_.available_clock_frequencies_)::value_type frequencies{};
            for (std::size_t i = 0; i < frequency_info.num_supported; ++i) {
                frequencies.push_back(static_cast<decltype(frequencies)::value_type>(frequency_info.frequency[i]) / 1000'000.0);
            }
            clock_samples_.available_clock_frequencies_ = frequencies;

            // queried samples -> retrieved every iteration if available
            clock_samples_.clock_frequency_ = decltype(clock_samples_.clock_frequency_)::value_type{};
            if (frequency_info.current < RSMI_MAX_NUM_FREQUENCIES) {
                clock_samples_.clock_frequency_->push_back(static_cast<decltype(clock_samples_.clock_frequency_)::value_type::value_type>(frequency_info.frequency[frequency_info.current]) / 1000'000.0);
            } else {
                clock_samples_.clock_frequency_->push_back(0);
            }
        }

        if (rsmi_dev_gpu_clk_freq_get(device_id_, RSMI_CLK_TYPE_SOC, &frequency_info) == RSMI_STATUS_SUCCESS) {
            clock_samples_.socket_clock_frequency_min_ = static_cast<decltype(clock_samples_.socket_clock_frequency_min_)::value_type>(frequency_info.frequency[0]) / 1000'000.0;
            clock_samples_.socket_clock_frequency_max_ = static_cast<decltype(clock_samples_.socket_clock_frequency_max_)::value_type>(frequency_info.frequency[frequency_info.num_supported - 1]) / 1000'000.0;
            // queried samples -> retrieved every iteration if available
            clock_samples_.socket_clock_frequency_ = decltype(clock_samples_.socket_clock_frequency_)::value_type{};
            if (frequency_info.current < RSMI_MAX_NUM_FREQUENCIES) {
                clock_samples_.socket_clock_frequency_->push_back(static_cast<decltype(clock_samples_.socket_clock_frequency_)::value_type::value_type>(frequency_info.frequency[frequency_info.current]) / 1000'000.0);
            } else {
                clock_samples_.socket_clock_frequency_->push_back(0);
            }
        }

        if (rsmi_dev_gpu_clk_freq_get(device_id_, RSMI_CLK_TYPE_MEM, &frequency_info) == RSMI_STATUS_SUCCESS) {
            clock_samples_.memory_clock_frequency_min_ = static_cast<decltype(clock_samples_.memory_clock_frequency_min_)::value_type>(frequency_info.frequency[0]) / 1000'000.0;
            clock_samples_.memory_clock_frequency_max_ = static_cast<decltype(clock_samples_.memory_clock_frequency_max_)::value_type>(frequency_info.frequency[frequency_info.num_supported - 1]) / 1000'000.0;
            decltype(clock_samples_.available_memory_clock_frequencies_)::value_type frequencies{};
            for (std::size_t i = 0; i < frequency_info.num_supported; ++i) {
                frequencies.push_back(static_cast<decltype(frequencies)::value_type>(frequency_info.frequency[i]) / 1000'000.0);
            }
            clock_samples_.available_memory_clock_frequencies_ = frequencies;

            // queried samples -> retrieved every iteration if available
            clock_samples_.memory_clock_frequency_ = decltype(clock_samples_.memory_clock_frequency_)::value_type{};
            if (frequency_info.current < RSMI_MAX_NUM_FREQUENCIES) {
                clock_samples_.memory_clock_frequency_->push_back(static_cast<decltype(clock_samples_.memory_clock_frequency_)::value_type::value_type>(frequency_info.frequency[frequency_info.current]) / 1000'000.0);
            } else {
                clock_samples_.memory_clock_frequency_->push_back(0);
            }
        }

        // queried samples -> retrieved every iteration if available
        decltype(clock_samples_.overdrive_level_)::value_type::value_type overdrive_level{};
        if (rsmi_dev_overdrive_level_get(device_id_, &overdrive_level) == RSMI_STATUS_SUCCESS) {
            clock_samples_.overdrive_level_ = decltype(clock_samples_.overdrive_level_)::value_type{ overdrive_level };
        }

        decltype(clock_samples_.memory_overdrive_level_)::value_type::value_type memory_overdrive_level{};
        if (rsmi_dev_mem_overdrive_level_get(device_id_, &memory_overdrive_level) == RSMI_STATUS_SUCCESS) {
            clock_samples_.memory_overdrive_level_ = decltype(clock_samples_.memory_overdrive_level_)::value_type{ memory_overdrive_level };
        }
    }

    // retrieve initial power related information
    if (this->sample_category_enabled(sample_category::power)) {
        std::uint64_t power_default_cap{};
        if (rsmi_dev_power_cap_default_get(device_id_, &power_default_cap) == RSMI_STATUS_SUCCESS) {
            power_samples_.power_management_limit_ = static_cast<decltype(power_samples_.power_management_limit_)::value_type>(power_default_cap) / 1000'000.0;
        }

        std::uint64_t power_cap{};
        if (rsmi_dev_power_cap_get(device_id_, std::uint32_t{ 0 }, &power_cap) == RSMI_STATUS_SUCCESS) {
            power_samples_.power_enforced_limit_ = static_cast<decltype(power_samples_.power_enforced_limit_)::value_type>(power_cap) / 1000'000.0;
        }

        {
            RSMI_POWER_TYPE power_type{};
            std::uint64_t power_usage{};
            if (rsmi_dev_power_get(device_id_, &power_usage, &power_type) == RSMI_STATUS_SUCCESS) {
                switch (power_type) {
                    case RSMI_POWER_TYPE::RSMI_AVERAGE_POWER:
                        power_samples_.power_measurement_type_ = "average";
                        break;
                    case RSMI_POWER_TYPE::RSMI_CURRENT_POWER:
                        power_samples_.power_measurement_type_ = "current/instant";
                        break;
                    case RSMI_POWER_TYPE::RSMI_INVALID_POWER:
                        power_samples_.power_measurement_type_ = "invalid/undetected";
                        break;
                }
                // report power usage since the first sample
                power_samples_.power_usage_ = decltype(power_samples_.power_usage_)::value_type{ static_cast<decltype(power_samples_.power_usage_)::value_type::value_type>(power_usage) / 1000'000.0 };
            }
        }

        rsmi_power_profile_status_t power_profile{};
        if (rsmi_dev_power_profile_presets_get(device_id_, std::uint32_t{ 0 }, &power_profile) == RSMI_STATUS_SUCCESS) {
            decltype(power_samples_.available_power_profiles_)::value_type available_power_profiles{};
            // go through all possible power profiles
            if ((power_profile.available_profiles & RSMI_PWR_PROF_PRST_CUSTOM_MASK) != std::uint64_t{ 0 }) {
                available_power_profiles.emplace_back("CUSTOM");
            }
            if ((power_profile.available_profiles & RSMI_PWR_PROF_PRST_VIDEO_MASK) != std::uint64_t{ 0 }) {
                available_power_profiles.emplace_back("VIDEO");
            }
            if ((power_profile.available_profiles & RSMI_PWR_PROF_PRST_POWER_SAVING_MASK) != std::uint64_t{ 0 }) {
                available_power_profiles.emplace_back("POWER_SAVING");
            }
            if ((power_profile.available_profiles & RSMI_PWR_PROF_PRST_COMPUTE_MASK) != std::uint64_t{ 0 }) {
                available_power_profiles.emplace_back("COMPUTE");
            }
            if ((power_profile.available_profiles & RSMI_PWR_PROF_PRST_VR_MASK) != std::uint64_t{ 0 }) {
                available_power_profiles.emplace_back("VR");
            }
            if ((power_profile.available_profiles & RSMI_PWR_PROF_PRST_3D_FULL_SCR_MASK) != std::uint64_t{ 0 }) {
                available_power_profiles.emplace_back("3D_FULL_SCREEN");
            }
            if ((power_profile.available_profiles & RSMI_PWR_PROF_PRST_BOOTUP_DEFAULT) != std::uint64_t{ 0 }) {
                available_power_profiles.emplace_back("BOOTUP_DEFAULT");
            }
            power_samples_.available_power_profiles_ = std::move(available_power_profiles);

            // queried samples -> retrieved every iteration if available
            switch (power_profile.current) {
                case RSMI_PWR_PROF_PRST_CUSTOM_MASK:
                    power_samples_.power_profile_ = decltype(power_samples_.power_profile_)::value_type{ "CUSTOM" };
                    break;
                case RSMI_PWR_PROF_PRST_VIDEO_MASK:
                    power_samples_.power_profile_ = decltype(power_samples_.power_profile_)::value_type{ "VIDEO" };
                    break;
                case RSMI_PWR_PROF_PRST_POWER_SAVING_MASK:
                    power_samples_.power_profile_ = decltype(power_samples_.power_profile_)::value_type{ "POWER_SAVING" };
                    break;
                case RSMI_PWR_PROF_PRST_COMPUTE_MASK:
                    power_samples_.power_profile_ = decltype(power_samples_.power_profile_)::value_type{ "COMPUTE" };
                    break;
                case RSMI_PWR_PROF_PRST_VR_MASK:
                    power_samples_.power_profile_ = decltype(power_samples_.power_profile_)::value_type{ "VR" };
                    break;
                case RSMI_PWR_PROF_PRST_3D_FULL_SCR_MASK:
                    power_samples_.power_profile_ = decltype(power_samples_.power_profile_)::value_type{ "3D_FULL_SCREEN" };
                    break;
                case RSMI_PWR_PROF_PRST_BOOTUP_DEFAULT:
                    power_samples_.power_profile_ = decltype(power_samples_.power_profile_)::value_type{ "BOOTUP_DEFAULT" };
                    break;
                case RSMI_PWR_PROF_PRST_INVALID:
                    power_samples_.power_profile_ = decltype(power_samples_.power_profile_)::value_type{ "INVALID" };
                    break;
            }
        }

        // queried samples -> retrieved every iteration if available
        [[maybe_unused]] std::uint64_t timestamp{};
        float resolution{};
        std::uint64_t power_total_energy_consumption{};
        if (rsmi_dev_energy_count_get(device_id_, &power_total_energy_consumption, &resolution, &timestamp) == RSMI_STATUS_SUCCESS) {
            const auto scaled_value = static_cast<decltype(power_samples_.power_total_energy_consumption_)::value_type::value_type>(power_total_energy_consumption) * static_cast<decltype(power_samples_.power_total_energy_consumption_)::value_type::value_type>(resolution);
            initial_total_power_consumption = scaled_value / 1000'000.0;
            power_samples_.power_total_energy_consumption_ = decltype(power_samples_.power_total_energy_consumption_)::value_type{ 0.0 };
        } else if (power_samples_.power_usage_.has_value()) {
            // if the total energy consumption cannot be retrieved, but the current power draw, approximate it
            power_samples_.power_total_energy_consumption_ = decltype(power_samples_.power_total_energy_consumption_)::value_type{ 0.0 };
        }
    }

    // retrieve initial memory related information
    if (this->sample_category_enabled(sample_category::memory)) {
        decltype(memory_samples_.memory_total_)::value_type memory_total{};
        if (rsmi_dev_memory_total_get(device_id_, RSMI_MEM_TYPE_VRAM, &memory_total) == RSMI_STATUS_SUCCESS) {
            memory_samples_.memory_total_ = memory_total;
        }

        decltype(memory_samples_.visible_memory_total_)::value_type visible_memory_total{};
        if (rsmi_dev_memory_total_get(device_id_, RSMI_MEM_TYPE_VIS_VRAM, &visible_memory_total) == RSMI_STATUS_SUCCESS) {
            memory_samples_.visible_memory_total_ = visible_memory_total;
        }

        rsmi_pcie_bandwidth_t bandwidth_info{};
        if (rsmi_dev_pci_bandwidth_get(device_id_, &bandwidth_info) == RSMI_STATUS_SUCCESS) {
            memory_samples_.num_pcie_lanes_min_ = bandwidth_info.lanes[0];
            memory_samples_.num_pcie_lanes_max_ = bandwidth_info.lanes[bandwidth_info.transfer_rate.num_supported - 1];
            memory_samples_.pcie_link_transfer_rate_min_ = bandwidth_info.transfer_rate.frequency[0] / 1'000'000;
            memory_samples_.pcie_link_transfer_rate_max_ = bandwidth_info.transfer_rate.frequency[bandwidth_info.transfer_rate.num_supported - 1] / 1'000'000;

            // queried samples -> retrieved every iteration if available
            memory_samples_.pcie_link_transfer_rate_ = decltype(memory_samples_.pcie_link_transfer_rate_)::value_type{};
            memory_samples_.num_pcie_lanes_ = decltype(memory_samples_.num_pcie_lanes_)::value_type{};
            if (bandwidth_info.transfer_rate.current < RSMI_MAX_NUM_FREQUENCIES) {
                memory_samples_.pcie_link_transfer_rate_->push_back(bandwidth_info.transfer_rate.frequency[bandwidth_info.transfer_rate.current] / 1'000'000);
                memory_samples_.num_pcie_lanes_->push_back(bandwidth_info.lanes[bandwidth_info.transfer_rate.current]);
            } else {
                // the current index is (somehow) wrong
                memory_samples_.pcie_link_transfer_rate_->push_back(0);
                memory_samples_.num_pcie_lanes_->push_back(0);
            }
        }

        // queried samples -> retrieved every iteration if available
        decltype(memory_samples_.memory_used_)::value_type::value_type memory_used{};
        if (rsmi_dev_memory_usage_get(device_id_, RSMI_MEM_TYPE_VRAM, &memory_used) == RSMI_STATUS_SUCCESS) {
            memory_samples_.memory_used_ = decltype(memory_samples_.memory_used_)::value_type{ memory_used };
            if (memory_samples_.memory_total_.has_value()) {
                memory_samples_.memory_free_ = decltype(memory_samples_.memory_used_)::value_type{ memory_samples_.memory_total_.value() - memory_samples_.memory_used_->front() };
            }
        }
    }

    // retrieve fixed temperature related information
    if (this->sample_category_enabled(sample_category::temperature)) {
        std::uint32_t fan_id{ 0 };
        std::int64_t fan_speed{};
        while (rsmi_dev_fan_speed_get(device_id_, fan_id, &fan_speed) == RSMI_STATUS_SUCCESS) {
            if (fan_id == 0) {
                // queried samples -> retrieved every iteration if available
                const auto percentage = static_cast<decltype(temperature_samples_.fan_speed_percentage_)::value_type::value_type>(fan_speed) / static_cast<decltype(temperature_samples_.fan_speed_percentage_)::value_type::value_type>(RSMI_MAX_FAN_SPEED);
                temperature_samples_.fan_speed_percentage_ = decltype(temperature_samples_.fan_speed_percentage_)::value_type{ percentage };
            }
            ++fan_id;
        }
        temperature_samples_.num_fans_ = fan_id;

        decltype(temperature_samples_.fan_speed_max_)::value_type max_fan_speed{};
        if (rsmi_dev_fan_speed_max_get(device_id_, std::uint32_t{ 0 }, &max_fan_speed) == RSMI_STATUS_SUCCESS) {
            temperature_samples_.fan_speed_max_ = max_fan_speed;
        }

        std::int64_t temperature_min{};
        if (rsmi_dev_temp_metric_get(device_id_, RSMI_TEMP_TYPE_EDGE, RSMI_TEMP_MIN, &temperature_min) == RSMI_STATUS_SUCCESS) {
            temperature_samples_.temperature_min_ = static_cast<decltype(temperature_samples_.temperature_min_)::value_type>(temperature_min) / 1000.0;
        }

        std::int64_t temperature_max{};
        if (rsmi_dev_temp_metric_get(device_id_, RSMI_TEMP_TYPE_EDGE, RSMI_TEMP_MAX, &temperature_max) == RSMI_STATUS_SUCCESS) {
            temperature_samples_.temperature_max_ = static_cast<decltype(temperature_samples_.temperature_max_)::value_type>(temperature_max) / 1000.0;
        }

        std::int64_t memory_temperature_min{};
        if (rsmi_dev_temp_metric_get(device_id_, RSMI_TEMP_TYPE_MEMORY, RSMI_TEMP_MIN, &memory_temperature_min) == RSMI_STATUS_SUCCESS) {
            temperature_samples_.memory_temperature_min_ = static_cast<decltype(temperature_samples_.memory_temperature_min_)::value_type>(memory_temperature_min) / 1000.0;
        }

        std::int64_t memory_temperature_max{};
        if (rsmi_dev_temp_metric_get(device_id_, RSMI_TEMP_TYPE_MEMORY, RSMI_TEMP_MAX, &memory_temperature_max) == RSMI_STATUS_SUCCESS) {
            temperature_samples_.memory_temperature_max_ = static_cast<decltype(temperature_samples_.memory_temperature_max_)::value_type>(memory_temperature_max) / 1000.0;
        }

        std::int64_t hotspot_temperature_min{};
        if (rsmi_dev_temp_metric_get(device_id_, RSMI_TEMP_TYPE_JUNCTION, RSMI_TEMP_MIN, &hotspot_temperature_min) == RSMI_STATUS_SUCCESS) {
            temperature_samples_.hotspot_temperature_min_ = static_cast<decltype(temperature_samples_.hotspot_temperature_min_)::value_type>(hotspot_temperature_min) / 1000.0;
        }

        std::int64_t hotspot_temperature_max{};
        if (rsmi_dev_temp_metric_get(device_id_, RSMI_TEMP_TYPE_JUNCTION, RSMI_TEMP_MAX, &hotspot_temperature_max) == RSMI_STATUS_SUCCESS) {
            temperature_samples_.hotspot_temperature_max_ = static_cast<decltype(temperature_samples_.hotspot_temperature_max_)::value_type>(hotspot_temperature_max) / 1000.0;
        }

        std::int64_t hbm_0_temperature_min{};
        if (rsmi_dev_temp_metric_get(device_id_, RSMI_TEMP_TYPE_HBM_0, RSMI_TEMP_MIN, &hbm_0_temperature_min) == RSMI_STATUS_SUCCESS) {
            temperature_samples_.hbm_0_temperature_min_ = static_cast<decltype(temperature_samples_.hbm_0_temperature_min_)::value_type>(hbm_0_temperature_min) / 1000.0;
        }

        std::int64_t hbm_0_temperature_max{};
        if (rsmi_dev_temp_metric_get(device_id_, RSMI_TEMP_TYPE_HBM_0, RSMI_TEMP_MAX, &hbm_0_temperature_max) == RSMI_STATUS_SUCCESS) {
            temperature_samples_.hbm_0_temperature_max_ = static_cast<decltype(temperature_samples_.hbm_0_temperature_max_)::value_type>(hbm_0_temperature_max) / 1000.0;
        }

        std::int64_t hbm_1_temperature_min{};
        if (rsmi_dev_temp_metric_get(device_id_, RSMI_TEMP_TYPE_HBM_1, RSMI_TEMP_MIN, &hbm_1_temperature_min) == RSMI_STATUS_SUCCESS) {
            temperature_samples_.hbm_1_temperature_min_ = static_cast<decltype(temperature_samples_.hbm_1_temperature_min_)::value_type>(hbm_1_temperature_min) / 1000.0;
        }

        std::int64_t hbm_1_temperature_max{};
        if (rsmi_dev_temp_metric_get(device_id_, RSMI_TEMP_TYPE_HBM_1, RSMI_TEMP_MAX, &hbm_1_temperature_max) == RSMI_STATUS_SUCCESS) {
            temperature_samples_.hbm_1_temperature_max_ = static_cast<decltype(temperature_samples_.hbm_1_temperature_max_)::value_type>(hbm_1_temperature_max) / 1000.0;
        }

        std::int64_t hbm_2_temperature_min{};
        if (rsmi_dev_temp_metric_get(device_id_, RSMI_TEMP_TYPE_HBM_2, RSMI_TEMP_MIN, &hbm_2_temperature_min) == RSMI_STATUS_SUCCESS) {
            temperature_samples_.hbm_2_temperature_min_ = static_cast<decltype(temperature_samples_.hbm_2_temperature_min_)::value_type>(hbm_2_temperature_min) / 1000.0;
        }

        std::int64_t hbm_2_temperature_max{};
        if (rsmi_dev_temp_metric_get(device_id_, RSMI_TEMP_TYPE_HBM_2, RSMI_TEMP_MAX, &hbm_2_temperature_max) == RSMI_STATUS_SUCCESS) {
            temperature_samples_.hbm_2_temperature_max_ = static_cast<decltype(temperature_samples_.hbm_2_temperature_max_)::value_type>(hbm_2_temperature_max) / 1000.0;
        }

        std::int64_t hbm_3_temperature_min{};
        if (rsmi_dev_temp_metric_get(device_id_, RSMI_TEMP_TYPE_HBM_3, RSMI_TEMP_MIN, &hbm_3_temperature_min) == RSMI_STATUS_SUCCESS) {
            temperature_samples_.hbm_3_temperature_min_ = static_cast<decltype(temperature_samples_.hbm_3_temperature_min_)::value_type>(hbm_3_temperature_min) / 1000.0;
        }

        std::int64_t hbm_3_temperature_max{};
        if (rsmi_dev_temp_metric_get(device_id_, RSMI_TEMP_TYPE_HBM_3, RSMI_TEMP_MAX, &hbm_3_temperature_max) == RSMI_STATUS_SUCCESS) {
            temperature_samples_.hbm_3_temperature_max_ = static_cast<decltype(temperature_samples_.hbm_3_temperature_max_)::value_type>(hbm_3_temperature_max) / 1000.0;
        }

        // queried samples -> retrieved every iteration if available
        std::int64_t temperature{};
        if (rsmi_dev_temp_metric_get(device_id_, RSMI_TEMP_TYPE_EDGE, RSMI_TEMP_CURRENT, &temperature) == RSMI_STATUS_SUCCESS) {
            temperature_samples_.temperature_ = decltype(temperature_samples_.temperature_)::value_type{ static_cast<decltype(temperature_samples_.temperature_)::value_type::value_type>(temperature) / 1000.0 };
        }

        std::int64_t hotspot_temperature{};
        if (rsmi_dev_temp_metric_get(device_id_, RSMI_TEMP_TYPE_JUNCTION, RSMI_TEMP_CURRENT, &hotspot_temperature) == RSMI_STATUS_SUCCESS) {
            temperature_samples_.hotspot_temperature_ = decltype(temperature_samples_.hotspot_temperature_)::value_type{ static_cast<decltype(temperature_samples_.hotspot_temperature_)::value_type::value_type>(hotspot_temperature) / 1000.0 };
        }

        std::int64_t memory_temperature{};
        if (rsmi_dev_temp_metric_get(device_id_, RSMI_TEMP_TYPE_MEMORY, RSMI_TEMP_CURRENT, &memory_temperature) == RSMI_STATUS_SUCCESS) {
            temperature_samples_.memory_temperature_ = decltype(temperature_samples_.memory_temperature_)::value_type{ static_cast<decltype(temperature_samples_.memory_temperature_)::value_type::value_type>(memory_temperature) / 1000.0 };
        }

        std::int64_t hbm_0_temperature{};
        if (rsmi_dev_temp_metric_get(device_id_, RSMI_TEMP_TYPE_HBM_0, RSMI_TEMP_CURRENT, &hbm_0_temperature) == RSMI_STATUS_SUCCESS) {
            temperature_samples_.hbm_0_temperature_ = decltype(temperature_samples_.hbm_0_temperature_)::value_type{ static_cast<decltype(temperature_samples_.hbm_0_temperature_)::value_type::value_type>(hbm_0_temperature) / 1000.0 };
        }

        std::int64_t hbm_1_temperature{};
        if (rsmi_dev_temp_metric_get(device_id_, RSMI_TEMP_TYPE_HBM_1, RSMI_TEMP_CURRENT, &hbm_1_temperature) == RSMI_STATUS_SUCCESS) {
            temperature_samples_.hbm_1_temperature_ = decltype(temperature_samples_.hbm_1_temperature_)::value_type{ static_cast<decltype(temperature_samples_.hbm_1_temperature_)::value_type::value_type>(hbm_1_temperature) / 1000.0 };
        }

        std::int64_t hbm_2_temperature{};
        if (rsmi_dev_temp_metric_get(device_id_, RSMI_TEMP_TYPE_HBM_2, RSMI_TEMP_CURRENT, &hbm_2_temperature) == RSMI_STATUS_SUCCESS) {
            temperature_samples_.hbm_2_temperature_ = decltype(temperature_samples_.hbm_2_temperature_)::value_type{ static_cast<decltype(temperature_samples_.hbm_2_temperature_)::value_type::value_type>(hbm_2_temperature) / 1000.0 };
        }

        std::int64_t hbm_3_temperature{};
        if (rsmi_dev_temp_metric_get(device_id_, RSMI_TEMP_TYPE_HBM_3, RSMI_TEMP_CURRENT, &hbm_3_temperature) == RSMI_STATUS_SUCCESS) {
            temperature_samples_.hbm_3_temperature_ = decltype(temperature_samples_.hbm_3_temperature_)::value_type{ static_cast<decltype(temperature_samples_.hbm_3_temperature_)::value_type::value_type>(hbm_3_temperature) / 1000.0 };
        }
    }

    //
    // loop until stop_sampling() is called
    //

    while (!this->has_sampling_stopped()) {
        // only sample values if the sampler currently isn't paused
        if (this->is_sampling()) {
            // add current time point
            this->add_time_point(std::chrono::steady_clock::now());

            // retrieve general samples
            if (this->sample_category_enabled(sample_category::general)) {
                if (general_samples_.performance_level_.has_value()) {
                    rsmi_dev_perf_level_t pstate{};
                    HWS_ROCM_SMI_ERROR_CHECK(rsmi_dev_perf_level_get(device_id_, &pstate))
                    general_samples_.performance_level_->push_back(detail::performance_level_to_string(pstate));
                }

                if (general_samples_.compute_utilization_.has_value()) {
                    decltype(general_samples_.compute_utilization_)::value_type::value_type value{};
                    HWS_ROCM_SMI_ERROR_CHECK(rsmi_dev_busy_percent_get(device_id_, &value))
                    general_samples_.compute_utilization_->push_back(value);
                }

                if (general_samples_.memory_utilization_.has_value()) {
                    decltype(general_samples_.memory_utilization_)::value_type::value_type value{};
                    HWS_ROCM_SMI_ERROR_CHECK(rsmi_dev_memory_busy_percent_get(device_id_, &value))
                    general_samples_.memory_utilization_->push_back(value);
                }
            }

            // retrieve clock related samples
            if (this->sample_category_enabled(sample_category::clock)) {
                if (clock_samples_.clock_frequency_.has_value()) {
                    rsmi_frequencies_t frequency_info{};
                    HWS_ROCM_SMI_ERROR_CHECK(rsmi_dev_gpu_clk_freq_get(device_id_, RSMI_CLK_TYPE_SYS, &frequency_info))
                    if (frequency_info.current < RSMI_MAX_NUM_FREQUENCIES) {
                        clock_samples_.clock_frequency_->push_back(static_cast<decltype(clock_samples_.clock_frequency_)::value_type::value_type>(frequency_info.frequency[frequency_info.current]) / 1000'000.0);
                    } else {
                        // the current index is (somehow) wrong
                        clock_samples_.clock_frequency_->push_back(0);
                    }
                }

                if (clock_samples_.socket_clock_frequency_.has_value()) {
                    rsmi_frequencies_t frequency_info{};
                    HWS_ROCM_SMI_ERROR_CHECK(rsmi_dev_gpu_clk_freq_get(device_id_, RSMI_CLK_TYPE_SOC, &frequency_info))
                    if (frequency_info.current < RSMI_MAX_NUM_FREQUENCIES) {
                        clock_samples_.socket_clock_frequency_->push_back(static_cast<decltype(clock_samples_.socket_clock_frequency_)::value_type::value_type>(frequency_info.frequency[frequency_info.current]) / 1000'000.0);
                    } else {
                        // the current index is (somehow) wrong
                        clock_samples_.socket_clock_frequency_->push_back(0);
                    }
                }

                if (clock_samples_.memory_clock_frequency_.has_value()) {
                    rsmi_frequencies_t frequency_info{};
                    HWS_ROCM_SMI_ERROR_CHECK(rsmi_dev_gpu_clk_freq_get(device_id_, RSMI_CLK_TYPE_MEM, &frequency_info))
                    if (frequency_info.current < RSMI_MAX_NUM_FREQUENCIES) {
                        clock_samples_.memory_clock_frequency_->push_back(static_cast<decltype(clock_samples_.memory_clock_frequency_)::value_type::value_type>(frequency_info.frequency[frequency_info.current]) / 1000'000.0);
                    } else {
                        // the current index is (somehow) wrong
                        clock_samples_.memory_clock_frequency_->push_back(0);
                    }
                }

                if (clock_samples_.overdrive_level_.has_value()) {
                    decltype(clock_samples_.overdrive_level_)::value_type::value_type value{};
                    HWS_ROCM_SMI_ERROR_CHECK(rsmi_dev_overdrive_level_get(device_id_, &value))
                    clock_samples_.overdrive_level_->push_back(value);
                }

                if (clock_samples_.memory_overdrive_level_.has_value()) {
                    decltype(clock_samples_.memory_overdrive_level_)::value_type::value_type value{};
                    HWS_ROCM_SMI_ERROR_CHECK(rsmi_dev_mem_overdrive_level_get(device_id_, &value))
                    clock_samples_.memory_overdrive_level_->push_back(value);
                }
            }

            // retrieve power related samples
            if (this->sample_category_enabled(sample_category::power)) {
                if (power_samples_.power_usage_.has_value()) {
                    [[maybe_unused]] RSMI_POWER_TYPE power_type{};
                    std::uint64_t value{};
                    HWS_ROCM_SMI_ERROR_CHECK(rsmi_dev_power_get(device_id_, &value, &power_type))
                    power_samples_.power_usage_->push_back(static_cast<decltype(power_samples_.power_usage_)::value_type::value_type>(value) / 1000'000.0);
                }

                if (power_samples_.power_total_energy_consumption_.has_value()) {
                    [[maybe_unused]] std::uint64_t timestamp{};
                    float resolution{};
                    std::uint64_t value{};
                    if (rsmi_dev_energy_count_get(device_id_, &value, &resolution, &timestamp) == RSMI_STATUS_SUCCESS) {
                        const auto scaled_value = static_cast<decltype(power_samples_.power_total_energy_consumption_)::value_type::value_type>(value) * static_cast<decltype(power_samples_.power_total_energy_consumption_)::value_type::value_type>(resolution);
                        power_samples_.power_total_energy_consumption_->push_back((scaled_value / 1000'000.0) - initial_total_power_consumption);
                    } else if (power_samples_.power_usage_.has_value()) {
                        // if the total energy consumption cannot be retrieved, but the current power draw, approximate it
                        const std::size_t num_time_points = this->sampling_time_points().size();
                        const auto time_difference = std::chrono::duration<double>(this->sampling_time_points()[num_time_points - 1] - this->sampling_time_points()[num_time_points - 2]).count();
                        const auto current = power_samples_.power_usage_->back() * time_difference;
                        power_samples_.power_total_energy_consumption_->push_back(power_samples_.power_total_energy_consumption_->back() + current);
                    }
                }

                if (power_samples_.power_profile_.has_value()) {
                    rsmi_power_profile_status_t power_profile{};
                    HWS_ROCM_SMI_ERROR_CHECK(rsmi_dev_power_profile_presets_get(device_id_, std::uint32_t{ 0 }, &power_profile))
                    switch (power_profile.current) {
                        case RSMI_PWR_PROF_PRST_CUSTOM_MASK:
                            power_samples_.power_profile_->emplace_back("CUSTOM");
                            break;
                        case RSMI_PWR_PROF_PRST_VIDEO_MASK:
                            power_samples_.power_profile_->emplace_back("VIDEO");
                            break;
                        case RSMI_PWR_PROF_PRST_POWER_SAVING_MASK:
                            power_samples_.power_profile_->emplace_back("POWER_SAVING");
                            break;
                        case RSMI_PWR_PROF_PRST_COMPUTE_MASK:
                            power_samples_.power_profile_->emplace_back("COMPUTE");
                            break;
                        case RSMI_PWR_PROF_PRST_VR_MASK:
                            power_samples_.power_profile_->emplace_back("VR");
                            break;
                        case RSMI_PWR_PROF_PRST_3D_FULL_SCR_MASK:
                            power_samples_.power_profile_->emplace_back("3D_FULL_SCREEN");
                            break;
                        case RSMI_PWR_PROF_PRST_BOOTUP_DEFAULT:
                            power_samples_.power_profile_->emplace_back("BOOTUP_DEFAULT");
                            break;
                        case RSMI_PWR_PROF_PRST_INVALID:
                            power_samples_.power_profile_->emplace_back("INVALID");
                            break;
                    }
                }
            }

            // retrieve memory related samples
            if (this->sample_category_enabled(sample_category::memory)) {
                if (memory_samples_.memory_used_.has_value()) {
                    decltype(memory_samples_.memory_used_)::value_type::value_type value{};
                    HWS_ROCM_SMI_ERROR_CHECK(rsmi_dev_memory_usage_get(device_id_, RSMI_MEM_TYPE_VRAM, &value))
                    memory_samples_.memory_used_->push_back(value);
                    if (memory_samples_.memory_free_.has_value()) {
                        memory_samples_.memory_free_->push_back(memory_samples_.memory_total_.value() - value);
                    }
                }

                if (memory_samples_.pcie_link_transfer_rate_.has_value() && memory_samples_.num_pcie_lanes_.has_value()) {
                    rsmi_pcie_bandwidth_t bandwidth_info{};
                    HWS_ROCM_SMI_ERROR_CHECK(rsmi_dev_pci_bandwidth_get(device_id_, &bandwidth_info))
                    if (bandwidth_info.transfer_rate.current < RSMI_MAX_NUM_FREQUENCIES) {
                        memory_samples_.pcie_link_transfer_rate_->push_back(bandwidth_info.transfer_rate.frequency[bandwidth_info.transfer_rate.current] / 1'000'000);
                        memory_samples_.num_pcie_lanes_->push_back(bandwidth_info.lanes[bandwidth_info.transfer_rate.current]);
                    } else {
                        // the current index is (somehow) wrong
                        memory_samples_.pcie_link_transfer_rate_->push_back(0);
                        memory_samples_.num_pcie_lanes_->push_back(0);
                    }
                }
            }

            // retrieve temperature related samples
            if (this->sample_category_enabled(sample_category::temperature)) {
                if (temperature_samples_.fan_speed_percentage_.has_value()) {
                    std::int64_t value{};
                    HWS_ROCM_SMI_ERROR_CHECK(rsmi_dev_fan_speed_get(device_id_, std::uint32_t{ 0 }, &value))
                    temperature_samples_.fan_speed_percentage_->push_back(static_cast<decltype(temperature_samples_.fan_speed_percentage_)::value_type::value_type>(value) / static_cast<decltype(temperature_samples_.fan_speed_percentage_)::value_type::value_type>(RSMI_MAX_FAN_SPEED));
                }

                if (temperature_samples_.temperature_.has_value()) {
                    std::int64_t value{};
                    HWS_ROCM_SMI_ERROR_CHECK(rsmi_dev_temp_metric_get(device_id_, RSMI_TEMP_TYPE_EDGE, RSMI_TEMP_CURRENT, &value))
                    temperature_samples_.temperature_->push_back(static_cast<decltype(temperature_samples_.temperature_)::value_type::value_type>(value) / 1000.0);
                }

                if (temperature_samples_.memory_temperature_.has_value()) {
                    std::int64_t value{};
                    HWS_ROCM_SMI_ERROR_CHECK(rsmi_dev_temp_metric_get(device_id_, RSMI_TEMP_TYPE_MEMORY, RSMI_TEMP_CURRENT, &value))
                    temperature_samples_.memory_temperature_->push_back(static_cast<decltype(temperature_samples_.memory_temperature_)::value_type::value_type>(value) / 1000.0);
                }

                if (temperature_samples_.hotspot_temperature_.has_value()) {
                    std::int64_t value{};
                    HWS_ROCM_SMI_ERROR_CHECK(rsmi_dev_temp_metric_get(device_id_, RSMI_TEMP_TYPE_JUNCTION, RSMI_TEMP_CURRENT, &value))
                    temperature_samples_.hotspot_temperature_->push_back(static_cast<decltype(temperature_samples_.hotspot_temperature_)::value_type::value_type>(value) / 1000.0);
                }

                if (temperature_samples_.hbm_0_temperature_.has_value()) {
                    std::int64_t value{};
                    HWS_ROCM_SMI_ERROR_CHECK(rsmi_dev_temp_metric_get(device_id_, RSMI_TEMP_TYPE_HBM_0, RSMI_TEMP_CURRENT, &value))
                    temperature_samples_.hbm_0_temperature_->push_back(static_cast<decltype(temperature_samples_.hbm_0_temperature_)::value_type::value_type>(value) / 1000.0);
                }

                if (temperature_samples_.hbm_1_temperature_.has_value()) {
                    std::int64_t value{};
                    HWS_ROCM_SMI_ERROR_CHECK(rsmi_dev_temp_metric_get(device_id_, RSMI_TEMP_TYPE_HBM_1, RSMI_TEMP_CURRENT, &value))
                    temperature_samples_.hbm_1_temperature_->push_back(static_cast<decltype(temperature_samples_.hbm_1_temperature_)::value_type::value_type>(value) / 1000.0);
                }

                if (temperature_samples_.hbm_2_temperature_.has_value()) {
                    std::int64_t value{};
                    HWS_ROCM_SMI_ERROR_CHECK(rsmi_dev_temp_metric_get(device_id_, RSMI_TEMP_TYPE_HBM_2, RSMI_TEMP_CURRENT, &value))
                    temperature_samples_.hbm_2_temperature_->push_back(static_cast<decltype(temperature_samples_.hbm_2_temperature_)::value_type::value_type>(value) / 1000.0);
                }

                if (temperature_samples_.hbm_3_temperature_.has_value()) {
                    std::int64_t value{};
                    HWS_ROCM_SMI_ERROR_CHECK(rsmi_dev_temp_metric_get(device_id_, RSMI_TEMP_TYPE_HBM_3, RSMI_TEMP_CURRENT, &value))
                    temperature_samples_.hbm_3_temperature_->push_back(static_cast<decltype(temperature_samples_.hbm_3_temperature_)::value_type::value_type>(value) / 1000.0);
                }
            }
        }

        // wait for the sampling interval to pass to retrieve the next sample
        std::this_thread::sleep_for(this->sampling_interval());
    }
}

std::string gpu_amd_hardware_sampler::device_identification() const {
    return fmt::format("gpu_amd_device_{}", device_id_);
}

std::string gpu_amd_hardware_sampler::samples_only_as_yaml_string() const {
    // check whether it's safe to generate the YAML entry
    if (this->is_sampling()) {
        throw std::runtime_error{ "Can't create the final YAML entry if the hardware sampler is still running!" };
    }

    return fmt::format("{}{}"
                       "{}{}"
                       "{}{}"
                       "{}{}"
                       "{}",
                       general_samples_.generate_yaml_string(),
                       general_samples_.has_samples() ? "\n" : "",
                       clock_samples_.generate_yaml_string(),
                       clock_samples_.has_samples() ? "\n" : "",
                       power_samples_.generate_yaml_string(),
                       power_samples_.has_samples() ? "\n" : "",
                       memory_samples_.generate_yaml_string(),
                       memory_samples_.has_samples() ? "\n" : "",
                       temperature_samples_.generate_yaml_string());
}

std::ostream &operator<<(std::ostream &out, const gpu_amd_hardware_sampler &sampler) {
    if (sampler.is_sampling()) {
        out.setstate(std::ios_base::failbit);
        return out;
    } else {
        return out << fmt::format("sampling interval: {}\n"
                                  "time points: [{}]\n\n"
                                  "general samples:\n{}\n\n"
                                  "clock samples:\n{}\n\n"
                                  "power samples:\n{}\n\n"
                                  "memory samples:\n{}\n\n"
                                  "temperature samples:\n{}",
                                  sampler.sampling_interval(),
                                  fmt::join(detail::time_points_to_epoch(sampler.sampling_time_points()), ", "),
                                  sampler.general_samples(),
                                  sampler.clock_samples(),
                                  sampler.power_samples(),
                                  sampler.memory_samples(),
                                  sampler.temperature_samples());
    }
}

}  // namespace hws
