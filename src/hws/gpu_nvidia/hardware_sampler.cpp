/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hws/gpu_nvidia/hardware_sampler.hpp"

#include "hws/gpu_nvidia/nvml_device_handle_impl.hpp"  // hws::detail::nvml_device_handle implementation
#include "hws/gpu_nvidia/nvml_samples.hpp"             // hws::{nvml_general_samples, nvml_clock_samples, nvml_power_samples, nvml_memory_samples, nvml_temperature_samples}
#include "hws/gpu_nvidia/utility.hpp"                  // HWS_NVML_ERROR_CHECK
#include "hws/hardware_sampler.hpp"                    // hws::hardware_sampler
#include "hws/sample_category.hpp"                     // hws::sample_category
#include "hws/utility.hpp"                             // hws::detail::time_points_to_epoch

#include "fmt/chrono.h"  // direct formatting of std::chrono types
#include "fmt/format.h"  // fmt::format
#include "fmt/ranges.h"  // fmt::join
#include "nvml.h"        // NVML runtime functions

#include <algorithm>  // std::min_element, std::sort, std::transform
#include <chrono>     // std::chrono::{steady_clock, duration_cast, milliseconds}
#include <cstddef>    // std::size_t
#include <exception>  // std::exception, std::terminate
#include <ios>        // std::ios_base
#include <iostream>   // std::cerr, std::endl
#include <numeric>    // std::iota
#include <optional>   // std::optional
#include <ostream>    // std::ostream
#include <stdexcept>  // std::runtime_error
#include <string>     // std::string
#include <thread>     // std::this_thread
#include <vector>     // std::vector

namespace hws {

gpu_nvidia_hardware_sampler::gpu_nvidia_hardware_sampler(const sample_category category) :
    gpu_nvidia_hardware_sampler{ 0, HWS_SAMPLING_INTERVAL, category } { }

gpu_nvidia_hardware_sampler::gpu_nvidia_hardware_sampler(const std::size_t device_id, const sample_category category) :
    gpu_nvidia_hardware_sampler{ device_id, HWS_SAMPLING_INTERVAL, category } { }

gpu_nvidia_hardware_sampler::gpu_nvidia_hardware_sampler(const std::chrono::milliseconds sampling_interval, const sample_category category) :
    gpu_nvidia_hardware_sampler{ 0, sampling_interval, category } { }

gpu_nvidia_hardware_sampler::gpu_nvidia_hardware_sampler(const std::size_t device_id, const std::chrono::milliseconds sampling_interval, const sample_category category) :
    hardware_sampler{ sampling_interval, category } {
    // make sure that nvmlInit is only called once for all instances
    if (instances_++ == 0) {
        HWS_NVML_ERROR_CHECK(nvmlInit())
        // notify that initialization has been finished
        init_finished_ = true;
    } else {
        // wait until init has been finished!
        while (!init_finished_) { }
    }

    // initialize samples -> can't be done beforehand since the device handle can only be initialized after a call to nvmlInit
    device_ = detail::nvml_device_handle{ device_id };
}

gpu_nvidia_hardware_sampler::~gpu_nvidia_hardware_sampler() {
    try {
        // if this hardware sampler is still sampling, stop it
        if (this->has_sampling_started() && !this->has_sampling_stopped()) {
            this->stop_sampling();
        }

        // the last instance must shut down the NVML runtime
        // make sure that nvmlShutdown is only called once
        if (--instances_ == 0) {
            HWS_NVML_ERROR_CHECK(nvmlShutdown())
            // reset init_finished flag
            init_finished_ = false;
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::terminate();
    }
}

void gpu_nvidia_hardware_sampler::sampling_loop() {
    // get the nvml handle from the device
    nvmlDevice_t device = device_.get_impl().device;

    //
    // add samples where we only have to retrieve the value once
    //

    this->add_time_point(std::chrono::steady_clock::now());

    double initial_total_power_consumption{};  // initial total power consumption in J

    // retrieve initial general information
    if (this->sample_category_enabled(sample_category::general)) {
        // fixed information -> only retrieved once
        nvmlDeviceArchitecture_t device_arch{};
        if (nvmlDeviceGetArchitecture(device, &device_arch) == NVML_SUCCESS) {
            switch (device_arch) {
#if defined(NVML_DEVICE_ARCH_KEPLER)
                case NVML_DEVICE_ARCH_KEPLER:
                    general_samples_.architecture_ = "Kepler";
                    break;
#endif
#if defined(NVML_DEVICE_ARCH_MAXWELL)
                case NVML_DEVICE_ARCH_MAXWELL:
                    general_samples_.architecture_ = "Maxwell";
                    break;
#endif
#if defined(NVML_DEVICE_ARCH_PASCAL)
                case NVML_DEVICE_ARCH_PASCAL:
                    general_samples_.architecture_ = "Pascal";
                    break;
#endif
#if defined(NVML_DEVICE_ARCH_VOLTA)
                case NVML_DEVICE_ARCH_VOLTA:
                    general_samples_.architecture_ = "Volta";
                    break;
#endif
#if defined(NVML_DEVICE_ARCH_TURING)
                case NVML_DEVICE_ARCH_TURING:
                    general_samples_.architecture_ = "Turing";
                    break;
#endif
#if defined(NVML_DEVICE_ARCH_AMPERE)
                case NVML_DEVICE_ARCH_AMPERE:
                    general_samples_.architecture_ = "Ampere";
                    break;
#endif
#if defined(NVML_DEVICE_ARCH_ADA)
                case NVML_DEVICE_ARCH_ADA:
                    general_samples_.architecture_ = "Ada";
                    break;
#endif
#if defined(NVML_DEVICE_ARCH_HOPPER)
                case NVML_DEVICE_ARCH_HOPPER:
                    general_samples_.architecture_ = "Hopper";
                    break;
#endif
#if defined(NVML_DEVICE_ARCH_BLACKWELL)
                case NVML_DEVICE_ARCH_BLACKWELL:
                    general_samples_.architecture_ = "Blackwell";
                    break;
#endif
#if defined(NVML_DEVICE_ARCH_T23X)
                case NVML_DEVICE_ARCH_T23X:
                    general_samples_.architecture_ = "Orin";
                    break;
#endif
                default:
                    break;
            }
        }

        // the byte order is given by the NVIDIA CUDA guide
        general_samples_.byte_order_ = "Little Endian";

        // the vendor ID is fixed for NVIDIA GPUs
        general_samples_.vendor_id_ = "NVIDIA";

        std::string name(NVML_DEVICE_NAME_V2_BUFFER_SIZE, '\0');
        if (nvmlDeviceGetName(device, name.data(), name.size()) == NVML_SUCCESS) {
            general_samples_.name_ = name.substr(0, name.find_first_of('\0'));
        }

        nvmlEnableState_t mode{};
        if (nvmlDeviceGetPersistenceMode(device, &mode) == NVML_SUCCESS) {
            general_samples_.persistence_mode_ = mode == NVML_FEATURE_ENABLED;
        }

        decltype(general_samples_.num_cores_)::value_type num_cores{};
        if (nvmlDeviceGetNumGpuCores(device, &num_cores) == NVML_SUCCESS) {
            general_samples_.num_cores_ = num_cores;
        }

        // queried samples -> retrieved every iteration if available
        nvmlPstates_t pstate{};
        if (nvmlDeviceGetPerformanceState(device, &pstate) == NVML_SUCCESS) {
            general_samples_.performance_level_ = decltype(general_samples_.performance_level_)::value_type{ static_cast<decltype(general_samples_.performance_level_)::value_type::value_type>(pstate) };
        }

        nvmlUtilization_t util{};
        if (nvmlDeviceGetUtilizationRates(device, &util) == NVML_SUCCESS) {
            general_samples_.compute_utilization_ = decltype(general_samples_.compute_utilization_)::value_type{ util.gpu };
            general_samples_.memory_utilization_ = decltype(general_samples_.memory_utilization_)::value_type{ util.memory };
        }
    }

    // retrieve initial clock related information
    if (this->sample_category_enabled(sample_category::clock)) {
        // fixed information -> only retrieved once
        unsigned int adaptive_clock_status{};
        if (nvmlDeviceGetAdaptiveClockInfoStatus(device, &adaptive_clock_status) == NVML_SUCCESS) {
            clock_samples_.auto_boosted_clock_enabled_ = adaptive_clock_status == NVML_ADAPTIVE_CLOCKING_INFO_STATUS_ENABLED;
        }

        unsigned int clock_graph_max{};
        if (nvmlDeviceGetMaxClockInfo(device, NVML_CLOCK_GRAPHICS, &clock_graph_max) == NVML_SUCCESS) {
            clock_samples_.clock_frequency_max_ = static_cast<decltype(clock_samples_.clock_frequency_max_)::value_type>(clock_graph_max);
        }

        unsigned int clock_sm_max{};
        if (nvmlDeviceGetMaxClockInfo(device, NVML_CLOCK_SM, &clock_sm_max) == NVML_SUCCESS) {
            clock_samples_.sm_clock_frequency_max_ = static_cast<decltype(clock_samples_.sm_clock_frequency_max_)::value_type>(clock_sm_max);
        }

        unsigned int clock_mem_max{};
        if (nvmlDeviceGetMaxClockInfo(device, NVML_CLOCK_MEM, &clock_mem_max) == NVML_SUCCESS) {
            clock_samples_.memory_clock_frequency_max_ = static_cast<decltype(clock_samples_.memory_clock_frequency_max_)::value_type>(clock_mem_max);
        }

        {
            unsigned int clock_count{ 128 };
            std::vector<unsigned int> supported_clocks(clock_count);
            if (nvmlDeviceGetSupportedMemoryClocks(device, &clock_count, supported_clocks.data()) == NVML_SUCCESS) {
                supported_clocks.resize(clock_count);
                clock_samples_.memory_clock_frequency_min_ = static_cast<decltype(clock_samples_.memory_clock_frequency_min_)::value_type>(*std::min_element(supported_clocks.cbegin(), supported_clocks.cend()));

                decltype(clock_samples_.available_memory_clock_frequencies_)::value_type available_memory_clock_frequencies(supported_clocks.size());
                // convert unsigned int values to double values
                std::transform(supported_clocks.cbegin(), supported_clocks.cend(), available_memory_clock_frequencies.begin(), [](const unsigned int c) { return static_cast<decltype(clock_samples_.available_memory_clock_frequencies_)::value_type::value_type>(c); });
                // we want to report all supported memory clocks in ascending order
                std::sort(available_memory_clock_frequencies.begin(), available_memory_clock_frequencies.end());
                clock_samples_.available_memory_clock_frequencies_ = available_memory_clock_frequencies;
            }
        }

        {
            unsigned int clock_count{ 128 };
            std::vector<unsigned int> supported_clocks(clock_count);
            if (clock_samples_.memory_clock_frequency_min_.has_value() && nvmlDeviceGetSupportedGraphicsClocks(device, static_cast<unsigned int>(clock_samples_.memory_clock_frequency_min_.value()), &clock_count, supported_clocks.data()) == NVML_SUCCESS) {
                clock_samples_.clock_frequency_min_ = static_cast<decltype(clock_samples_.clock_frequency_min_)::value_type>(*std::min_element(supported_clocks.cbegin(), supported_clocks.cbegin() + clock_count));
            }

            if (clock_samples_.available_memory_clock_frequencies_.has_value()) {
                for (const auto value : clock_samples_.available_memory_clock_frequencies_.value()) {
                    if (nvmlDeviceGetSupportedGraphicsClocks(device, static_cast<unsigned int>(value), &clock_count, supported_clocks.data()) == NVML_SUCCESS) {
                        decltype(clock_samples_.available_clock_frequencies_)::value_type::mapped_type available_clock_frequencies(clock_count);
                        // convert unsigned int values to double values
                        std::transform(supported_clocks.cbegin(), supported_clocks.cbegin() + clock_count, available_clock_frequencies.begin(), [](const unsigned int c) { return static_cast<decltype(clock_samples_.available_clock_frequencies_)::value_type::mapped_type::value_type>(c); });
                        // we want to report all supported memory clocks in ascending order
                        std::sort(available_clock_frequencies.begin(), available_clock_frequencies.end());
                        // if no map exists, default construct an empty map
                        if (!clock_samples_.available_clock_frequencies_.has_value()) {
                            clock_samples_.available_clock_frequencies_ = decltype(clock_samples_)::map_type{};
                        }
                        clock_samples_.available_clock_frequencies_->emplace(value, available_clock_frequencies);
                    }
                }
            }
        }

        // queried samples -> retrieved every iteration if available
        unsigned int clock_graph{};
        if (nvmlDeviceGetClockInfo(device, NVML_CLOCK_GRAPHICS, &clock_graph) == NVML_SUCCESS) {
            clock_samples_.clock_frequency_ = decltype(clock_samples_.clock_frequency_)::value_type{ static_cast<decltype(clock_samples_.clock_frequency_)::value_type::value_type>(clock_graph) };
        }

        unsigned int clock_sm{};
        if (nvmlDeviceGetClockInfo(device, NVML_CLOCK_SM, &clock_sm) == NVML_SUCCESS) {
            clock_samples_.sm_clock_frequency_ = decltype(clock_samples_.sm_clock_frequency_)::value_type{ static_cast<decltype(clock_samples_.sm_clock_frequency_)::value_type::value_type>(clock_sm) };
        }

        unsigned int clock_mem{};
        if (nvmlDeviceGetClockInfo(device, NVML_CLOCK_MEM, &clock_mem) == NVML_SUCCESS) {
            clock_samples_.memory_clock_frequency_ = decltype(clock_samples_.memory_clock_frequency_)::value_type{ static_cast<decltype(clock_samples_.memory_clock_frequency_)::value_type::value_type>(clock_mem) };
        }

#if CUDA_VERSION >= 12000
        decltype(clock_samples_.throttle_reason_)::value_type::value_type clock_throttle_reason{};
        if (nvmlDeviceGetCurrentClocksEventReasons(device, &clock_throttle_reason) == NVML_SUCCESS) {
            clock_samples_.throttle_reason_ = decltype(clock_samples_.throttle_reason_)::value_type{ clock_throttle_reason };
            clock_samples_.throttle_reason_string_ = decltype(clock_samples_.throttle_reason_string_)::value_type{ detail::throttle_event_reason_to_string(clock_throttle_reason) };
        }
#endif

        nvmlEnableState_t mode{};
        nvmlEnableState_t default_mode{};
        if (nvmlDeviceGetAutoBoostedClocksEnabled(device, &mode, &default_mode) == NVML_SUCCESS) {
            clock_samples_.auto_boosted_clock_ = decltype(clock_samples_.auto_boosted_clock_)::value_type{ mode == NVML_FEATURE_ENABLED };
        }
    }

    // retrieve initial power related information
    if (this->sample_category_enabled(sample_category::power)) {
        // fixed information -> only retrieved once
        nvmlEnableState_t mode{};
        if (nvmlDeviceGetPowerManagementMode(device, &mode) == NVML_SUCCESS) {
            power_samples_.power_management_mode_ = mode == NVML_FEATURE_ENABLED;
        }

        unsigned int power_management_limit{};
        if (nvmlDeviceGetPowerManagementLimit(device, &power_management_limit) == NVML_SUCCESS) {
            power_samples_.power_management_limit_ = static_cast<decltype(power_samples_.power_management_limit_)::value_type>(power_management_limit) / 1000.0;
        }

        unsigned int power_enforced_limit{};
        if (nvmlDeviceGetEnforcedPowerLimit(device, &power_enforced_limit) == NVML_SUCCESS) {
            power_samples_.power_enforced_limit_ = static_cast<decltype(power_samples_.power_enforced_limit_)::value_type>(power_enforced_limit) / 1000.0;
        }

        if (general_samples_.architecture_.has_value()) {
            // based on https://docs.nvidia.com/deploy/nvml-api/group__nvmlDeviceQueries.html#group__nvmlDeviceQueries_1gf754f109beca3a4a8c8c1cd650d7d66c
            if (general_samples_.architecture_ == "Kepler" || general_samples_.architecture_ == "Maxwell" || general_samples_.architecture_ == "Pascal" || general_samples_.architecture_ == "Volta" || general_samples_.architecture_ == "Turing") {
                power_samples_.power_measurement_type_ = "current/instant";
            } else if (general_samples_.architecture_ == "Ampere" || general_samples_.architecture_ == "Ada" || general_samples_.architecture_ == "Hopper" || general_samples_.architecture_ == "Blackwell" || general_samples_.architecture_ == "Orin") {
                if (general_samples_.name_.has_value() && general_samples_.name_.value().find("A100") != std::string::npos) {
                    // GA100 also has instant power draw measurements
                    power_samples_.power_measurement_type_ = "current/instant";
                } else {
                    power_samples_.power_measurement_type_ = "average";
                }
            } else {
                power_samples_.power_measurement_type_ = "invalid/undetected";
            }
        }

        decltype(power_samples_.available_power_profiles_)::value_type power_states(17, 32);  // 17 power states, value 32 = unknown
        std::iota(power_states.begin(), power_states.end() - 1, decltype(power_samples_.available_power_profiles_)::value_type::value_type{ 0 });
        power_samples_.available_power_profiles_ = power_states;

        // queried samples -> retrieved every iteration if available
        unsigned int power_usage{};
        if (nvmlDeviceGetPowerUsage(device, &power_usage) == NVML_SUCCESS) {
            power_samples_.power_usage_ = decltype(power_samples_.power_usage_)::value_type{ static_cast<decltype(power_samples_.power_usage_)::value_type::value_type>(power_usage) / 1000.0 };
        }

        unsigned long long power_total_energy_consumption{};
        if (nvmlDeviceGetTotalEnergyConsumption(device, &power_total_energy_consumption) == NVML_SUCCESS) {
            initial_total_power_consumption = static_cast<decltype(power_samples_.power_total_energy_consumption_)::value_type::value_type>(power_total_energy_consumption) / 1000.0;
            power_samples_.power_total_energy_consumption_ = decltype(power_samples_.power_total_energy_consumption_)::value_type{ 0.0 };
        }

        nvmlPstates_t pstate{};
        if (nvmlDeviceGetPowerState(device, &pstate) == NVML_SUCCESS) {
            power_samples_.power_profile_ = decltype(power_samples_.power_profile_)::value_type{ static_cast<decltype(power_samples_.power_profile_)::value_type::value_type>(pstate) };
        }

        nvmlFieldValue_t system_power{};
        system_power.fieldId = NVML_FI_DEV_POWER_AVERAGE;
        system_power.scopeId = 1;
        if (nvmlDeviceGetFieldValues(device, 1, &system_power) == NVML_SUCCESS) {
            power_samples_.system_power_usage_ = decltype(power_samples_.system_power_usage_)::value_type{ static_cast<decltype(power_samples_.system_power_usage_)::value_type::value_type>(system_power.value.uiVal) / 1000.0 };
        }
    }

    // retrieve initial memory related information
    if (this->sample_category_enabled(sample_category::memory)) {
        // fixed information -> only retrieved once
        nvmlMemory_t memory_info{};
        if (nvmlDeviceGetMemoryInfo(device, &memory_info) == NVML_SUCCESS) {
            memory_samples_.memory_total_ = memory_info.total;
            // queried samples -> retrieved every iteration if available
            memory_samples_.memory_free_ = decltype(memory_samples_.memory_free_)::value_type{ memory_info.free };
            memory_samples_.memory_used_ = decltype(memory_samples_.memory_used_)::value_type{ memory_info.used };
        }

        decltype(memory_samples_.memory_bus_width_)::value_type memory_bus_width{};
        if (nvmlDeviceGetMemoryBusWidth(device, &memory_bus_width) == NVML_SUCCESS) {
            memory_samples_.memory_bus_width_ = memory_bus_width;
        }

        decltype(memory_samples_.num_pcie_lanes_max_)::value_type num_pcie_lanes_max{};
        if (nvmlDeviceGetMaxPcieLinkWidth(device, &num_pcie_lanes_max) == NVML_SUCCESS) {
            memory_samples_.num_pcie_lanes_max_ = num_pcie_lanes_max;
        }

        decltype(memory_samples_.pcie_link_generation_max_)::value_type pcie_link_generation_max{};
        if (nvmlDeviceGetMaxPcieLinkGeneration(device, &pcie_link_generation_max) == NVML_SUCCESS) {
            memory_samples_.pcie_link_generation_max_ = pcie_link_generation_max;
        }

        decltype(memory_samples_.pcie_link_speed_max_)::value_type pcie_link_speed_max{};
        if (nvmlDeviceGetPcieLinkMaxSpeed(device, &pcie_link_speed_max) == NVML_SUCCESS) {
            memory_samples_.pcie_link_speed_max_ = pcie_link_speed_max;
        }

        // queried samples -> retrieved every iteration if available
        decltype(memory_samples_.num_pcie_lanes_)::value_type::value_type num_pcie_lanes{};
        if (nvmlDeviceGetCurrPcieLinkWidth(device, &num_pcie_lanes) == NVML_SUCCESS) {
            memory_samples_.num_pcie_lanes_ = decltype(memory_samples_.num_pcie_lanes_)::value_type{ num_pcie_lanes };
        }

        decltype(memory_samples_.pcie_link_generation_)::value_type::value_type pcie_link_generation{};
        if (nvmlDeviceGetCurrPcieLinkGeneration(device, &pcie_link_generation) == NVML_SUCCESS) {
            memory_samples_.pcie_link_generation_ = decltype(memory_samples_.pcie_link_generation_)::value_type{ pcie_link_generation };
        }
    }

    // retrieve initial temperature related information
    if (this->sample_category_enabled(sample_category::temperature)) {
        // fixed information -> only retrieved once
        decltype(temperature_samples_.num_fans_)::value_type num_fans{};
        if (nvmlDeviceGetNumFans(device, &num_fans) == NVML_SUCCESS) {
            temperature_samples_.num_fans_ = num_fans;
        }

        if (temperature_samples_.num_fans_.has_value() && temperature_samples_.num_fans_.value() > 0) {
            decltype(temperature_samples_.fan_speed_min_)::value_type min_fan_speed{};
            decltype(temperature_samples_.fan_speed_max_)::value_type max_fan_speed{};
            if (nvmlDeviceGetMinMaxFanSpeed(device, &min_fan_speed, &max_fan_speed) == NVML_SUCCESS) {
                temperature_samples_.fan_speed_min_ = min_fan_speed;
                temperature_samples_.fan_speed_max_ = max_fan_speed;
            }
        }

        unsigned int temperature_max{};
        if (nvmlDeviceGetTemperatureThreshold(device, NVML_TEMPERATURE_THRESHOLD_GPU_MAX, &temperature_max) == NVML_SUCCESS) {
            temperature_samples_.temperature_max_ = static_cast<decltype(temperature_samples_.temperature_max_)::value_type>(temperature_max);
        }

        unsigned int memory_temperature_max{};
        if (nvmlDeviceGetTemperatureThreshold(device, NVML_TEMPERATURE_THRESHOLD_MEM_MAX, &memory_temperature_max) == NVML_SUCCESS) {
            temperature_samples_.memory_temperature_max_ = static_cast<decltype(temperature_samples_.memory_temperature_max_)::value_type>(memory_temperature_max);
        }

        // queried samples -> retrieved every iteration if available
        unsigned int fan_speed_percentage{};
        if (nvmlDeviceGetFanSpeed(device, &fan_speed_percentage) == NVML_SUCCESS) {
            temperature_samples_.fan_speed_percentage_ = decltype(temperature_samples_.fan_speed_percentage_)::value_type{ static_cast<decltype(temperature_samples_.fan_speed_percentage_)::value_type::value_type>(fan_speed_percentage) };
        }

        unsigned int temperature{};
        if (nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &temperature) == NVML_SUCCESS) {
            temperature_samples_.temperature_ = decltype(temperature_samples_.temperature_)::value_type{ static_cast<decltype(temperature_samples_.temperature_)::value_type::value_type>(temperature) };
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
                    nvmlPstates_t pstate{};
                    HWS_NVML_ERROR_CHECK(nvmlDeviceGetPerformanceState(device, &pstate))
                    general_samples_.performance_level_->push_back(static_cast<decltype(general_samples_.performance_level_)::value_type::value_type>(pstate));
                }

                if (general_samples_.compute_utilization_.has_value() && general_samples_.memory_utilization_.has_value()) {
                    nvmlUtilization_t util{};
                    HWS_NVML_ERROR_CHECK(nvmlDeviceGetUtilizationRates(device, &util))
                    general_samples_.compute_utilization_->push_back(util.gpu);
                    general_samples_.memory_utilization_->push_back(util.memory);
                }
            }

            // retrieve clock related samples
            if (this->sample_category_enabled(sample_category::clock)) {
                if (clock_samples_.clock_frequency_.has_value()) {
                    unsigned int value{};
                    HWS_NVML_ERROR_CHECK(nvmlDeviceGetClockInfo(device, NVML_CLOCK_GRAPHICS, &value))
                    clock_samples_.clock_frequency_->push_back(static_cast<decltype(clock_samples_.clock_frequency_)::value_type::value_type>(value));
                }

                if (clock_samples_.sm_clock_frequency_.has_value()) {
                    unsigned int value{};
                    HWS_NVML_ERROR_CHECK(nvmlDeviceGetClockInfo(device, NVML_CLOCK_SM, &value))
                    clock_samples_.sm_clock_frequency_->push_back(static_cast<decltype(clock_samples_.sm_clock_frequency_)::value_type::value_type>(value));
                }

                if (clock_samples_.memory_clock_frequency_.has_value()) {
                    unsigned int value{};
                    HWS_NVML_ERROR_CHECK(nvmlDeviceGetClockInfo(device, NVML_CLOCK_MEM, &value))
                    clock_samples_.memory_clock_frequency_->push_back(static_cast<decltype(clock_samples_.memory_clock_frequency_)::value_type::value_type>(value));
                }

#if CUDA_VERSION >= 12000
                if (clock_samples_.throttle_reason_string_.has_value()) {
                    decltype(clock_samples_.throttle_reason_)::value_type::value_type value{};
                    HWS_NVML_ERROR_CHECK(nvmlDeviceGetCurrentClocksEventReasons(device, &value))
                    clock_samples_.throttle_reason_->push_back(value);
                    clock_samples_.throttle_reason_string_->push_back(detail::throttle_event_reason_to_string(value));
                }
#endif

                if (clock_samples_.auto_boosted_clock_.has_value()) {
                    nvmlEnableState_t mode{};
                    nvmlEnableState_t default_mode{};
                    HWS_NVML_ERROR_CHECK(nvmlDeviceGetAutoBoostedClocksEnabled(device, &mode, &default_mode))
                    clock_samples_.auto_boosted_clock_->push_back(mode == NVML_FEATURE_ENABLED);
                }
            }

            // retrieve power related information
            if (this->sample_category_enabled(sample_category::power)) {
                if (power_samples_.power_profile_.has_value()) {
                    nvmlPstates_t pstate{};
                    HWS_NVML_ERROR_CHECK(nvmlDeviceGetPowerState(device, &pstate))
                    power_samples_.power_profile_->push_back(static_cast<decltype(power_samples_.power_profile_)::value_type::value_type>(pstate));
                }

                if (power_samples_.power_usage_.has_value()) {
                    unsigned int value{};
                    HWS_NVML_ERROR_CHECK(nvmlDeviceGetPowerUsage(device, &value))
                    power_samples_.power_usage_->push_back(static_cast<decltype(power_samples_.power_usage_)::value_type::value_type>(value) / 1000.0);
                }

                if (power_samples_.system_power_usage_.has_value()) {
                    nvmlFieldValue_t system_power{};
                    system_power.fieldId = NVML_FI_DEV_POWER_AVERAGE;  // 185
                    system_power.scopeId = 1;                          // scope for full module, not just GPU
                    HWS_NVML_ERROR_CHECK(nvmlDeviceGetFieldValues(device, 1, &system_power))
                    power_samples_.system_power_usage_->push_back(static_cast<decltype(power_samples_.system_power_usage_)::value_type::value_type>(system_power.value.uiVal) / 1000.0);
                }

                if (power_samples_.power_total_energy_consumption_.has_value()) {
                    unsigned long long value{};
                    HWS_NVML_ERROR_CHECK(nvmlDeviceGetTotalEnergyConsumption(device, &value))
                    power_samples_.power_total_energy_consumption_->push_back((static_cast<decltype(power_samples_.power_total_energy_consumption_)::value_type::value_type>(value) / 1000.0) - initial_total_power_consumption);
                }
            }

            // retrieve memory related information
            if (this->sample_category_enabled(sample_category::memory)) {
                if (memory_samples_.memory_free_.has_value() && memory_samples_.memory_used_.has_value()) {
                    nvmlMemory_t memory_info{};
                    HWS_NVML_ERROR_CHECK(nvmlDeviceGetMemoryInfo(device, &memory_info))
                    memory_samples_.memory_free_->push_back(memory_info.free);
                    memory_samples_.memory_used_->push_back(memory_info.used);
                }

                if (memory_samples_.num_pcie_lanes_.has_value()) {
                    decltype(memory_samples_.num_pcie_lanes_)::value_type::value_type value{};
                    HWS_NVML_ERROR_CHECK(nvmlDeviceGetCurrPcieLinkWidth(device, &value))
                    memory_samples_.num_pcie_lanes_->push_back(value);
                }

                if (memory_samples_.pcie_link_generation_.has_value()) {
                    decltype(memory_samples_.pcie_link_generation_)::value_type::value_type value{};
                    HWS_NVML_ERROR_CHECK(nvmlDeviceGetCurrPcieLinkGeneration(device, &value))
                    memory_samples_.pcie_link_generation_->push_back(value);
                }
            }

            // retrieve temperature related information
            if (this->sample_category_enabled(sample_category::temperature)) {
                if (temperature_samples_.fan_speed_percentage_.has_value()) {
                    unsigned int value{};
                    HWS_NVML_ERROR_CHECK(nvmlDeviceGetFanSpeed(device, &value))
                    temperature_samples_.fan_speed_percentage_->push_back(static_cast<decltype(temperature_samples_.fan_speed_percentage_)::value_type::value_type>(value));
                }

                if (temperature_samples_.temperature_.has_value()) {
                    unsigned int value{};
                    HWS_NVML_ERROR_CHECK(nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &value))
                    temperature_samples_.temperature_->push_back(static_cast<decltype(temperature_samples_.temperature_)::value_type::value_type>(value));
                }
            }
        }

        // wait for the sampling interval to pass to retrieve the next sample
        std::this_thread::sleep_for(this->sampling_interval());
    }
}

std::string gpu_nvidia_hardware_sampler::device_identification() const {
    nvmlPciInfo_st pcie_info{};
    HWS_NVML_ERROR_CHECK(nvmlDeviceGetPciInfo_v3(device_.get_impl().device, &pcie_info))
    return fmt::format("gpu_nvidia_device_{}_{}", pcie_info.device, pcie_info.bus);
}

std::string gpu_nvidia_hardware_sampler::samples_only_as_yaml_string() const {
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

std::ostream &operator<<(std::ostream &out, const gpu_nvidia_hardware_sampler &sampler) {
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
