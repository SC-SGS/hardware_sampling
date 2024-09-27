/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hardware_sampling/gpu_intel/hardware_sampler.hpp"

#include "hardware_sampling/gpu_intel/level_zero_device_handle_impl.hpp"  // hws::level_zero_device_handle implementation
#include "hardware_sampling/gpu_intel/level_zero_samples.hpp"             // hws::{level_zero_general_samples, level_zero_clock_samples, level_zero_power_samples, level_zero_memory_samples, level_zero_temperature_samples}
#include "hardware_sampling/gpu_intel/utility.hpp"                        // HWS_LEVEL_ZERO_ERROR_CHECK
#include "hardware_sampling/hardware_sampler.hpp"                         // hws::hardware_sampler
#include "hardware_sampling/sample_category.hpp"                          // hws::sample_category
#include "hardware_sampling/utility.hpp"                                  // hws::{durations_from_reference_time, join}

#include "fmt/format.h"          // fmt::format
#include "level_zero/ze_api.h"   // Level Zero runtime functions
#include "level_zero/zes_api.h"  // Level Zero runtime functions

#include <chrono>     // std::chrono::{steady_clock, duration_cast, milliseconds}
#include <cstddef>    // std::size_t
#include <cstdint>    // std::int32_t, std::int64_t
#include <exception>  // std::exception, std::terminate
#include <ios>        // std::ios_base
#include <iostream>   // std::cerr, std::endl
#include <stdexcept>  // std::runtime_error
#include <string>     // std::string
#include <thread>     // std::this_thread
#include <utility>    // std::move
#include <vector>     // std::vector

namespace hws {

gpu_intel_hardware_sampler::gpu_intel_hardware_sampler(const sample_category category) :
    gpu_intel_hardware_sampler{ 0, HWS_SAMPLING_INTERVAL, category } { }

gpu_intel_hardware_sampler::gpu_intel_hardware_sampler(const std::size_t device_id, const sample_category category) :
    gpu_intel_hardware_sampler{ device_id, HWS_SAMPLING_INTERVAL, category } { }

gpu_intel_hardware_sampler::gpu_intel_hardware_sampler(const std::chrono::milliseconds sampling_interval, const sample_category category) :
    gpu_intel_hardware_sampler{ 0, sampling_interval, category } { }

gpu_intel_hardware_sampler::gpu_intel_hardware_sampler(const std::size_t device_id, const std::chrono::milliseconds sampling_interval, const sample_category category) :
    hardware_sampler{ sampling_interval, category } {
    // make sure that zeInit is only called once for all instances
    if (instances_++ == 0) {
        HWS_LEVEL_ZERO_ERROR_CHECK(zeInit(ZE_INIT_FLAG_GPU_ONLY))
        // notify that initialization has been finished
        init_finished_ = true;
    } else {
        // wait until init has been finished!
        while (!init_finished_) { }
    }

    // initialize samples -> can't be done beforehand since the device handle can only be initialized after a call to nvmlInit
    device_ = detail::level_zero_device_handle{ device_id };
}

gpu_intel_hardware_sampler::~gpu_intel_hardware_sampler() {
    try {
        // if this hardware sampler is still sampling, stop it
        if (this->has_sampling_started() && !this->has_sampling_stopped()) {
            this->stop_sampling();
        }
        // the level zero runtime has no dedicated shut down or cleanup function
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::terminate();
    }
}

void gpu_intel_hardware_sampler::sampling_loop() {
    // get the level zero handle from the device
    ze_device_handle_t device = device_.get_impl().device;

    // the different handles
    std::vector<zes_freq_handle_t> frequency_handles{};
    std::vector<zes_pwr_handle_t> power_handles{};
    std::vector<zes_mem_handle_t> memory_handles{};
    std::vector<zes_fan_handle_t> fan_handles{};
    std::vector<zes_psu_handle_t> psu_handles{};
    std::vector<zes_temp_handle_t> temperature_handles{};

    //
    // add samples where we only have to retrieve the value once
    //

    this->add_time_point(std::chrono::steady_clock::now());

    double initial_total_power_consumption{};  // initial total power consumption in J

    // retrieve initial general information
    if (this->sample_category_enabled(sample_category::general)) {
        // the byte order is given by Intel directly
        general_samples_.byte_order_ = "Little Endian";

        ze_device_properties_t ze_device_prop{};
        if (zeDeviceGetProperties(device, &ze_device_prop) == ZE_RESULT_SUCCESS) {
            general_samples_.vendor_id_ = fmt::format("{:x}", ze_device_prop.vendorId);
            general_samples_.num_threads_per_eu_ = ze_device_prop.numThreadsPerEU;
            general_samples_.eu_simd_width_ = ze_device_prop.physicalEUSimdWidth;

            // assemble list of GPU flags
            general_samples_.flags_ = detail::property_flags_to_vector(ze_device_prop.flags);
        }

        zes_device_properties_t zes_device_prop{};
        if (zesDeviceGetProperties(device, &zes_device_prop) == ZE_RESULT_SUCCESS) {
            general_samples_.name_ = zes_device_prop.modelName;
        }

        std::uint32_t num_standby_domains{ 0 };
        if (zesDeviceEnumStandbyDomains(device, &num_standby_domains, nullptr) == ZE_RESULT_SUCCESS) {
            std::vector<zes_standby_handle_t> standby_handles(num_standby_domains);
            if (zesDeviceEnumStandbyDomains(device, &num_standby_domains, standby_handles.data()) == ZE_RESULT_SUCCESS) {
                if (!standby_handles.empty()) {
                    // NOTE: only the first standby domain is used here
                    zes_standby_promo_mode_t mode{};
                    if (zesStandbyGetMode(standby_handles.front(), &mode) == ZE_RESULT_SUCCESS) {
                        std::string standby_mode_name{ "unknown" };
                        switch (mode) {
                            case ZES_STANDBY_PROMO_MODE_DEFAULT:
                                standby_mode_name = "default";
                                break;
                            case ZES_STANDBY_PROMO_MODE_NEVER:
                                standby_mode_name = "never";
                                break;
                            default:
                                // do nothing
                                break;
                        }
                        general_samples_.standby_mode_ = std::move(standby_mode_name);
                    }
                }
            }
        }
    }

    // retrieve initial clock related information
    if (this->sample_category_enabled(sample_category::clock)) {
        std::uint32_t num_frequency_domains{ 0 };
        if (zesDeviceEnumFrequencyDomains(device, &num_frequency_domains, nullptr) == ZE_RESULT_SUCCESS) {
            frequency_handles.resize(num_frequency_domains);
            if (zesDeviceEnumFrequencyDomains(device, &num_frequency_domains, frequency_handles.data()) == ZE_RESULT_SUCCESS) {
                for (zes_freq_handle_t handle : frequency_handles) {
                    // get frequency properties
                    zes_freq_properties_t prop{};
                    if (zesFrequencyGetProperties(handle, &prop)) {
                        // determine the frequency domain (e.g. GPU, memory, etc)
                        switch (prop.type) {
                            case ZES_FREQ_DOMAIN_GPU:
                                clock_samples_.clock_frequency_min_ = prop.min;
                                clock_samples_.clock_frequency_max_ = prop.max;
                                break;
                            case ZES_FREQ_DOMAIN_MEMORY:
                                clock_samples_.memory_clock_frequency_min_ = prop.min;
                                clock_samples_.memory_clock_frequency_max_ = prop.max;
                                break;
                            default:
                                // do nothing
                                break;
                        }

                        // get possible frequencies
                        std::uint32_t num_available_clocks{ 0 };
                        if (zesFrequencyGetAvailableClocks(handle, &num_available_clocks, nullptr) == ZE_RESULT_SUCCESS) {
                            std::vector<double> available_clocks(num_available_clocks);
                            if (zesFrequencyGetAvailableClocks(handle, &num_available_clocks, available_clocks.data()) == ZE_RESULT_SUCCESS) {
                                // determine the frequency domain (e.g. GPU, memory, etc)
                                switch (prop.type) {
                                    case ZES_FREQ_DOMAIN_GPU:
                                        clock_samples_.available_clock_frequencies_ = available_clocks;
                                        break;
                                    case ZES_FREQ_DOMAIN_MEMORY:
                                        clock_samples_.available_memory_clock_frequencies_ = available_clocks;
                                        break;
                                    default:
                                        // do nothing
                                        break;
                                }
                            }
                        }

                        // get current frequency information
                        zes_freq_state_t frequency_state{};
                        if (zesFrequencyGetState(handle, &frequency_state) == ZE_RESULT_SUCCESS) {
                            // determine the frequency domain (e.g. GPU, memory, etc)
                            switch (prop.type) {
                                case ZES_FREQ_DOMAIN_GPU:
                                    {
                                        if (frequency_state.tdp >= 0.0) {
                                            clock_samples_.frequency_limit_tdp_ = decltype(clock_samples_.frequency_limit_tdp_)::value_type{ frequency_state.tdp };
                                        }
                                        if (frequency_state.actual >= 0.0) {
                                            clock_samples_.clock_frequency_ = decltype(clock_samples_.clock_frequency_)::value_type{ frequency_state.actual };
                                        }
                                        if (frequency_state.throttleReasons >= 0.0) {
                                            {
                                                using vector_type = decltype(clock_samples_.throttle_reason_)::value_type;
                                                clock_samples_.throttle_reason_ = vector_type{ static_cast<vector_type::value_type>(static_cast<std::int64_t>(frequency_state.throttleReasons)) };
                                            }
                                            {
                                                using vector_type = decltype(clock_samples_.throttle_reason_string_)::value_type;
                                                clock_samples_.throttle_reason_string_ = vector_type{ static_cast<vector_type::value_type>(detail::throttle_reason_to_string(frequency_state.throttleReasons)) };
                                            }
                                        }
                                    }
                                    break;
                                case ZES_FREQ_DOMAIN_MEMORY:
                                    {
                                        if (frequency_state.tdp >= 0.0) {
                                            clock_samples_.memory_frequency_limit_tdp_ = decltype(clock_samples_.memory_frequency_limit_tdp_)::value_type{ frequency_state.tdp };
                                        }
                                        if (frequency_state.actual >= 0.0) {
                                            clock_samples_.memory_clock_frequency_ = decltype(clock_samples_.memory_clock_frequency_)::value_type{ frequency_state.actual };
                                        }
                                        if (frequency_state.throttleReasons >= 0.0) {
                                            {
                                                using vector_type = decltype(clock_samples_.memory_throttle_reason_)::value_type;
                                                clock_samples_.memory_throttle_reason_ = vector_type{ static_cast<vector_type::value_type>(static_cast<std::int64_t>(frequency_state.throttleReasons)) };
                                            }
                                            {
                                                using vector_type = decltype(clock_samples_.memory_throttle_reason_string_)::value_type;
                                                clock_samples_.memory_throttle_reason_string_ = vector_type{ static_cast<vector_type::value_type>(detail::throttle_reason_to_string(frequency_state.throttleReasons)) };
                                            }
                                        }
                                    }
                                    break;
                                default:
                                    // do nothing
                                    break;
                            }
                        }
                    }
                }
            }
        }
    }

    // retrieve initial power related information
    if (this->sample_category_enabled(sample_category::power)) {
        std::uint32_t num_power_domains{ 0 };
        if (zesDeviceEnumPowerDomains(device, &num_power_domains, nullptr) == ZE_RESULT_SUCCESS) {
            power_handles.resize(num_power_domains);
            if (zesDeviceEnumPowerDomains(device, &num_power_domains, power_handles.data()) == ZE_RESULT_SUCCESS) {
                if (!power_handles.empty()) {
                    // NOTE: only the first power domain is used here
                    // get the power measurement type
                    // NOTE: only the first value is used here!
                    std::uint32_t num_power_limit_descriptors{ 1 };
                    zes_power_limit_ext_desc_t desc{};
                    if (zesPowerGetLimitsExt(power_handles.front(), &num_power_limit_descriptors, &desc) == ZE_RESULT_SUCCESS) {
                        switch (desc.level) {
                            case ZES_POWER_LEVEL_UNKNOWN:
                                power_samples_.power_measurement_type_ = "unknown";
                                break;
                            case ZES_POWER_LEVEL_SUSTAINED:
                                power_samples_.power_measurement_type_ = "sustained";
                                break;
                            case ZES_POWER_LEVEL_BURST:
                                power_samples_.power_measurement_type_ = "burst";
                                break;
                            case ZES_POWER_LEVEL_PEAK:
                                power_samples_.power_measurement_type_ = "peak";
                                break;
                            case ZES_POWER_LEVEL_INSTANTANEOUS:
                                power_samples_.power_measurement_type_ = "current/instant";
                                break;
                            case ZES_POWER_LEVEL_FORCE_UINT32:
                                power_samples_.power_measurement_type_ = "force uint32";
                                break;
                        }

                        power_samples_.power_enforced_limit_ = static_cast<decltype(power_samples_.power_enforced_limit_)::value_type>(desc.limit);
                    }

                    // get total power consumption
                    zes_power_energy_counter_t energy_counter{};
                    if (zesPowerGetEnergyCounter(power_handles.front(), &energy_counter) == ZE_RESULT_SUCCESS) {
                        initial_total_power_consumption = static_cast<decltype(power_samples_.power_total_energy_consumption_)::value_type::value_type>(energy_counter.energy) / 1000.0 / 1000.0;
                        power_samples_.power_total_energy_consumption_ = decltype(power_samples_.power_total_energy_consumption_)::value_type{ 0.0 };
                        power_samples_.power_usage_ = decltype(power_samples_.power_total_energy_consumption_)::value_type{ 0.0 };
                    }

                    // get energy thresholds
                    zes_energy_threshold_t energy_threshold{};
                    if (zesPowerGetEnergyThreshold(power_handles.front(), &energy_threshold) == ZE_RESULT_SUCCESS) {
                        power_samples_.power_management_mode_ = static_cast<decltype(power_samples_.power_management_mode_)::value_type>(energy_threshold.enable);
                    }
                }
            }
        }
    }

    // retrieve initial memory related information
    if (this->sample_category_enabled(sample_category::memory)) {
        std::uint32_t num_memory_modules{ 0 };
        if (zesDeviceEnumMemoryModules(device, &num_memory_modules, nullptr) == ZE_RESULT_SUCCESS) {
            memory_handles.resize(num_memory_modules);
            if (zesDeviceEnumMemoryModules(device, &num_memory_modules, memory_handles.data()) == ZE_RESULT_SUCCESS) {
                for (zes_mem_handle_t handle : memory_handles) {
                    zes_mem_properties_t prop{};
                    if (zesMemoryGetProperties(handle, &prop) == ZE_RESULT_SUCCESS) {
                        // get the memory module name
                        const std::string memory_module_name = detail::memory_module_to_name(prop.type);

                        if (prop.physicalSize > 0) {
                            // first value to add -> initialize map
                            if (!memory_samples_.memory_total_.has_value()) {
                                memory_samples_.memory_total_ = decltype(memory_samples_.memory_total_)::value_type{};
                            }
                            // add new physical size
                            memory_samples_.memory_total_.value()[memory_module_name] = prop.physicalSize;
                        }
                        if (prop.busWidth != -1) {
                            // first value to add -> initialize map
                            if (!memory_samples_.memory_bus_width_.has_value()) {
                                memory_samples_.memory_bus_width_ = decltype(memory_samples_.memory_bus_width_)::value_type{};
                            }
                            // add new memory bus width
                            memory_samples_.memory_bus_width_.value()[memory_module_name] = prop.busWidth;
                        }
                        if (prop.numChannels != -1) {
                            // first value to add -> initialize map
                            if (!memory_samples_.memory_num_channels_.has_value()) {
                                memory_samples_.memory_num_channels_ = decltype(memory_samples_.memory_num_channels_)::value_type{};
                            }
                            // add new number of memory channels
                            memory_samples_.memory_num_channels_.value()[memory_module_name] = prop.numChannels;
                        }
                        // first value to add -> initialize map
                        if (!memory_samples_.memory_location_.has_value()) {
                            memory_samples_.memory_location_ = decltype(memory_samples_.memory_location_)::value_type{};
                        }
                        memory_samples_.memory_location_.value()[memory_module_name] = detail::memory_location_to_name(prop.location);

                        // get current memory information
                        zes_mem_state_t mem_state{};
                        if (zesMemoryGetState(handle, &mem_state) == ZE_RESULT_SUCCESS) {
                            // first value to add -> initialize map
                            if (!memory_samples_.visible_memory_total_.has_value()) {
                                memory_samples_.visible_memory_total_ = decltype(memory_samples_.visible_memory_total_)::value_type{};
                            }
                            memory_samples_.visible_memory_total_.value()[memory_module_name] = mem_state.size;

                            // first value to add -> initialize map
                            if (!memory_samples_.memory_free_.has_value()) {
                                memory_samples_.memory_free_ = decltype(memory_samples_.memory_free_)::value_type{};
                            }
                            memory_samples_.memory_free_.value()[memory_module_name].push_back(mem_state.free);

                            // first value to add -> initialize map
                            if (!memory_samples_.memory_used_.has_value()) {
                                memory_samples_.memory_used_ = decltype(memory_samples_.memory_used_)::value_type{};
                            }
                            memory_samples_.memory_used_.value()[memory_module_name].push_back(mem_state.size - mem_state.free);
                        }
                    }
                }

                // the maximum PCIe stats
                zes_pci_properties_t pci_prop{};
                if (zesDevicePciGetProperties(device, &pci_prop) == ZE_RESULT_SUCCESS) {
                    if (pci_prop.maxSpeed.gen != -1) {
                        memory_samples_.pcie_link_generation_max_ = pci_prop.maxSpeed.gen;
                    }
                    if (pci_prop.maxSpeed.width != -1) {
                        memory_samples_.num_pcie_lanes_max_ = pci_prop.maxSpeed.width;
                    }
                    if (pci_prop.maxSpeed.maxBandwidth != -1) {
                        memory_samples_.pcie_link_speed_max_ = static_cast<decltype(memory_samples_.pcie_link_speed_max_)::value_type>(static_cast<double>(pci_prop.maxSpeed.maxBandwidth) / 1e6);
                    }
                }

                // the current PCIe stats
                zes_pci_state_t pci_state{};
                if (zesDevicePciGetState(device, &pci_state) == ZE_RESULT_SUCCESS) {
                    if (pci_state.speed.maxBandwidth != -1) {
                        memory_samples_.pcie_link_speed_ = decltype(memory_samples_.pcie_link_speed_)::value_type{ static_cast<decltype(memory_samples_.pcie_link_speed_max_)::value_type>(static_cast<double>(pci_state.speed.maxBandwidth) / 1e6) };
                    }
                    if (pci_state.speed.width != -1) {
                        memory_samples_.num_pcie_lanes_ = decltype(memory_samples_.num_pcie_lanes_)::value_type{ pci_state.speed.width };
                    }
                    if (pci_state.speed.gen != -1) {
                        memory_samples_.pcie_link_generation_ = decltype(memory_samples_.pcie_link_generation_)::value_type{ pci_state.speed.gen };
                    }
                }
            }
        }
    }

    // retrieve initial temperature related information
    if (this->sample_category_enabled(sample_category::temperature)) {
        std::uint32_t num_fans{ 0 };
        if (zesDeviceEnumFans(device, &num_fans, nullptr) == ZE_RESULT_SUCCESS) {
            temperature_samples_.num_fans_ = num_fans;

            fan_handles.resize(num_fans);
            if (zesDeviceEnumFans(device, &num_fans, fan_handles.data()) == ZE_RESULT_SUCCESS) {
                // NOTE: only the first fan handle is used here
                if (!fan_handles.empty()) {
                    zes_fan_properties_t prop{};
                    if (zesFanGetProperties(fan_handles.front(), &prop) == ZE_RESULT_SUCCESS) {
                        temperature_samples_.fan_speed_max_ = prop.maxRPM;
                    }

                    std::int32_t fan_speed{};
                    if (zesFanGetState(fan_handles.front(), ZES_FAN_SPEED_UNITS_PERCENT, &fan_speed) == ZE_RESULT_SUCCESS) {
                        if (fan_speed != -1) {
                            temperature_samples_.fan_speed_percentage_ = decltype(temperature_samples_.fan_speed_percentage_)::value_type{ static_cast<decltype(temperature_samples_.fan_speed_percentage_)::value_type::value_type>(fan_speed) };
                        }
                    }
                }
            }
        }

        std::uint32_t num_psus{ 0 };
        if (zesDeviceEnumPsus(device, &num_psus, nullptr) == ZE_RESULT_SUCCESS) {
            psu_handles.resize(num_psus);
            if (zesDeviceEnumPsus(device, &num_psus, psu_handles.data()) == ZE_RESULT_SUCCESS) {
                if (!psu_handles.empty()) {
                    // NOTE: only the first PSU is used here
                    zes_psu_state_t psu_state{};
                    if (zesPsuGetState(psu_handles.front(), &psu_state) == ZE_RESULT_SUCCESS) {
                        if (psu_state.temperature != -1) {
                            temperature_samples_.psu_temperature_ = static_cast<decltype(temperature_samples_.psu_temperature_)::value_type>(psu_state.temperature);
                        }
                    }
                }
            }
        }

        std::uint32_t num_temperature_sensors{ 0 };
        if (zesDeviceEnumTemperatureSensors(device, &num_temperature_sensors, nullptr) == ZE_RESULT_SUCCESS) {
            temperature_handles.resize(num_temperature_sensors);
            if (zesDeviceEnumTemperatureSensors(device, &num_temperature_sensors, temperature_handles.data()) == ZE_RESULT_SUCCESS) {
                for (zes_temp_handle_t handle : temperature_handles) {
                    zes_temp_properties_t prop{};
                    if (zesTemperatureGetProperties(handle, &prop) == ZE_RESULT_SUCCESS) {
                        switch (prop.type) {
                            case ZES_TEMP_SENSORS_GLOBAL:
                                {
                                    // first value to add -> initialize map
                                    if (!temperature_samples_.global_temperature_max_.has_value()) {
                                        temperature_samples_.global_temperature_max_ = decltype(temperature_samples_.global_temperature_max_)::value_type{};
                                    }
                                    // add new maximum temperature
                                    temperature_samples_.global_temperature_max_ = prop.maxTemperature;

                                    // first value to add -> initialize map
                                    if (!temperature_samples_.global_temperature_.has_value()) {
                                        temperature_samples_.global_temperature_ = decltype(temperature_samples_.global_temperature_)::value_type{};
                                    }
                                    double temp{};
                                    if (zesTemperatureGetState(handle, &temp) == ZE_RESULT_SUCCESS) {
                                        temperature_samples_.global_temperature_->push_back(temp);
                                    }
                                }
                                break;
                            case ZES_TEMP_SENSORS_GPU:
                                {
                                    // first value to add -> initialize map
                                    if (!temperature_samples_.temperature_max_.has_value()) {
                                        temperature_samples_.temperature_max_ = decltype(temperature_samples_.temperature_max_)::value_type{};
                                    }
                                    // add new maximum temperature
                                    temperature_samples_.temperature_max_ = prop.maxTemperature;

                                    // first value to add -> initialize map
                                    if (!temperature_samples_.temperature_.has_value()) {
                                        temperature_samples_.temperature_ = decltype(temperature_samples_.temperature_)::value_type{};
                                    }
                                    double temp{};
                                    if (zesTemperatureGetState(handle, &temp) == ZE_RESULT_SUCCESS) {
                                        temperature_samples_.temperature_->push_back(temp);
                                    }
                                }
                                break;
                            case ZES_TEMP_SENSORS_MEMORY:
                                {
                                    // first value to add -> initialize map
                                    if (!temperature_samples_.memory_temperature_max_.has_value()) {
                                        temperature_samples_.memory_temperature_max_ = decltype(temperature_samples_.memory_temperature_max_)::value_type{};
                                    }
                                    // add new maximum temperature
                                    temperature_samples_.memory_temperature_max_ = prop.maxTemperature;

                                    // first value to add -> initialize map
                                    if (!temperature_samples_.memory_temperature_.has_value()) {
                                        temperature_samples_.memory_temperature_ = decltype(temperature_samples_.memory_temperature_)::value_type{};
                                    }
                                    double temp{};
                                    if (zesTemperatureGetState(handle, &temp) == ZE_RESULT_SUCCESS) {
                                        temperature_samples_.memory_temperature_->push_back(temp);
                                    }
                                }
                                break;
                            default:
                                break;
                        }
                    }
                }
            }
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

            // retrieve clock related samples
            if (this->sample_category_enabled(sample_category::clock)) {
                for (zes_freq_handle_t handle : frequency_handles) {
                    // get frequency properties
                    zes_freq_properties_t prop{};
                    HWS_LEVEL_ZERO_ERROR_CHECK(zesFrequencyGetProperties(handle, &prop))

                    // get current frequency information
                    zes_freq_state_t frequency_state{};
                    if (clock_samples_.clock_frequency_.has_value() || clock_samples_.memory_clock_frequency_.has_value()) {
                        HWS_LEVEL_ZERO_ERROR_CHECK(zesFrequencyGetState(handle, &frequency_state))
                        // determine the frequency domain (e.g. GPU, memory, etc)
                        switch (prop.type) {
                            case ZES_FREQ_DOMAIN_GPU:
                                {
                                    if (clock_samples_.frequency_limit_tdp_.has_value()) {
                                        clock_samples_.frequency_limit_tdp_->push_back(frequency_state.tdp);
                                    }
                                    if (clock_samples_.clock_frequency_.has_value()) {
                                        clock_samples_.clock_frequency_->push_back(frequency_state.actual);
                                    }
                                    if (clock_samples_.throttle_reason_.has_value()) {
                                        clock_samples_.throttle_reason_->push_back(static_cast<std::int64_t>(frequency_state.throttleReasons));
                                    }
                                    if (clock_samples_.throttle_reason_string_.has_value()) {
                                        clock_samples_.throttle_reason_string_->push_back(detail::throttle_reason_to_string(frequency_state.throttleReasons));
                                    }
                                }
                                break;
                            case ZES_FREQ_DOMAIN_MEMORY:
                                {
                                    if (clock_samples_.memory_frequency_limit_tdp_.has_value()) {
                                        clock_samples_.memory_frequency_limit_tdp_->push_back(frequency_state.tdp);
                                    }
                                    if (clock_samples_.memory_clock_frequency_.has_value()) {
                                        clock_samples_.memory_clock_frequency_->push_back(frequency_state.actual);
                                    }
                                    if (clock_samples_.memory_throttle_reason_.has_value()) {
                                        clock_samples_.memory_throttle_reason_->push_back(static_cast<std::int64_t>(frequency_state.throttleReasons));
                                    }
                                    if (clock_samples_.memory_throttle_reason_string_.has_value()) {
                                        clock_samples_.memory_throttle_reason_string_->push_back(detail::throttle_reason_to_string(frequency_state.throttleReasons));
                                    }
                                }
                                break;
                            default:
                                // do nothing
                                break;
                        }
                    }
                }
            }

            // retrieve power related samples
            if (this->sample_category_enabled(sample_category::power)) {
                if (!power_handles.empty()) {
                    // NOTE: only the first power domain is used here
                    if (power_samples_.power_total_energy_consumption_.has_value()) {
                        // get total power consumption
                        zes_power_energy_counter_t energy_counter{};
                        HWS_LEVEL_ZERO_ERROR_CHECK(zesPowerGetEnergyCounter(power_handles.front(), &energy_counter))

                        const auto power_consumption = static_cast<decltype(power_samples_.power_total_energy_consumption_)::value_type::value_type>(energy_counter.energy) / 1000.0 / 1000.0;

                        // calculate current power draw as (Energy Difference [J]) / (Time Difference [s])
                        const std::size_t last_index = this->sampling_time_points().size() - 1;
                        const double power_usage = (power_consumption - power_samples_.power_total_energy_consumption_->back()) / (std::chrono::duration<double>(this->sampling_time_points()[last_index] - this->sampling_time_points()[last_index - 1]).count());
                        power_samples_.power_usage_->push_back(power_usage);

                        // add power consumption last to be able to use the std::vector::back() function
                        power_samples_.power_total_energy_consumption_->push_back(power_consumption - initial_total_power_consumption);
                    }
                }
            }

            // retrieve memory related samples
            if (this->sample_category_enabled(sample_category::memory)) {
                for (zes_mem_handle_t handle : memory_handles) {
                    zes_mem_properties_t prop{};
                    HWS_LEVEL_ZERO_ERROR_CHECK(zesMemoryGetProperties(handle, &prop))

                    // get the memory module name
                    const std::string memory_module_name = detail::memory_module_to_name(prop.type);

                    if (memory_samples_.memory_free_.has_value()) {
                        // get current memory information
                        zes_mem_state_t mem_state{};
                        HWS_LEVEL_ZERO_ERROR_CHECK(zesMemoryGetState(handle, &mem_state))

                        memory_samples_.memory_free_.value()[memory_module_name].push_back(mem_state.free);

                        if (memory_samples_.visible_memory_total_.has_value()) {
                            memory_samples_.memory_used_.value()[memory_module_name].push_back(memory_samples_.visible_memory_total_.value()[memory_module_name] - mem_state.free);
                        }
                    }
                }

                if (memory_samples_.pcie_link_speed_.has_value() || memory_samples_.num_pcie_lanes_.has_value() || memory_samples_.num_pcie_lanes_.has_value()) {
                    // the current PCIe stats
                    zes_pci_state_t pci_state{};
                    HWS_LEVEL_ZERO_ERROR_CHECK(zesDevicePciGetState(device, &pci_state))
                    if (memory_samples_.pcie_link_speed_.has_value()) {
                        memory_samples_.pcie_link_speed_->push_back(static_cast<decltype(memory_samples_.pcie_link_speed_)::value_type::value_type>(static_cast<double>(pci_state.speed.maxBandwidth) / 1e6));
                    }
                    if (memory_samples_.num_pcie_lanes_.has_value()) {
                        memory_samples_.num_pcie_lanes_->push_back(pci_state.speed.width);
                    }
                    if (memory_samples_.pcie_link_generation_.has_value()) {
                        memory_samples_.pcie_link_generation_->push_back(pci_state.speed.gen);
                    }
                }
            }

            // retrieve temperature related samples
            if (this->sample_category_enabled(sample_category::temperature)) {
                if (!psu_handles.empty()) {
                    if (temperature_samples_.psu_temperature_.has_value()) {
                        // NOTE: only the first PSU is used here
                        zes_psu_state_t psu_state{};
                        HWS_LEVEL_ZERO_ERROR_CHECK(zesPsuGetState(psu_handles.front(), &psu_state))
                        temperature_samples_.psu_temperature_->push_back(psu_state.temperature);
                    }
                }

                for (zes_temp_handle_t handle : temperature_handles) {
                    zes_temp_properties_t prop{};
                    HWS_LEVEL_ZERO_ERROR_CHECK(zesTemperatureGetProperties(handle, &prop))

                    switch (prop.type) {
                        case ZES_TEMP_SENSORS_GLOBAL:
                            {
                                if (temperature_samples_.global_temperature_.has_value()) {
                                    double temp{};
                                    HWS_LEVEL_ZERO_ERROR_CHECK(zesTemperatureGetState(handle, &temp))
                                    temperature_samples_.global_temperature_->push_back(temp);
                                }
                            }
                            break;
                        case ZES_TEMP_SENSORS_GPU:
                            {
                                if (temperature_samples_.temperature_.has_value()) {
                                    double temp{};
                                    HWS_LEVEL_ZERO_ERROR_CHECK(zesTemperatureGetState(handle, &temp))
                                    temperature_samples_.temperature_->push_back(temp);
                                }
                            }
                            break;
                        case ZES_TEMP_SENSORS_MEMORY:
                            {
                                if (temperature_samples_.memory_temperature_.has_value()) {
                                    double temp{};
                                    HWS_LEVEL_ZERO_ERROR_CHECK(zesTemperatureGetState(handle, &temp))
                                    temperature_samples_.memory_temperature_->push_back(temp);
                                }
                            }
                            break;
                        default:
                            break;
                    }
                }
            }
        }

        // wait for the sampling interval to pass to retrieve the next sample
        std::this_thread::sleep_for(this->sampling_interval());
    }
}

std::string gpu_intel_hardware_sampler::device_identification() const {
    // get the level zero handle from the device
    ze_device_handle_t device = device_.get_impl().device;
    ze_device_properties_t prop{};
    HWS_LEVEL_ZERO_ERROR_CHECK(zeDeviceGetProperties(device, &prop))
    return fmt::format("gpu_intel_device_{}", prop.deviceId);
}

std::string gpu_intel_hardware_sampler::samples_only_as_yaml_string() const {
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

std::ostream &operator<<(std::ostream &out, const gpu_intel_hardware_sampler &sampler) {
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
