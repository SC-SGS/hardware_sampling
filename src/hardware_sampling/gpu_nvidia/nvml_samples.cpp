/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hardware_sampling/gpu_nvidia/nvml_samples.hpp"

#include "hardware_sampling/utility.hpp"  // hws::detail::{value_or_default, map_entry_to_string}

#include "fmt/format.h"  // fmt::format
#include "fmt/ranges.h"  // fmt::join

#include <ostream>  // std::ostream
#include <string>   // std::string

namespace hws {

//*************************************************************************************************************************************//
//                                                           general samples                                                           //
//*************************************************************************************************************************************//

std::string nvml_general_samples::generate_yaml_string() const {
    std::string str{ "general:\n" };

    // device architecture
    if (this->architecture_.has_value()) {
        str += fmt::format("  architecture:\n"
                           "    unit: \"string\"\n"
                           "    values: \"{}\"\n",
                           this->architecture_.value());
    }
    // device byte order
    if (this->byte_order_.has_value()) {
        str += fmt::format("  byte_order:\n"
                           "    unit: \"string\"\n"
                           "    values: \"{}\"\n",
                           this->byte_order_.value());
    }
    // the vendor specific ID
    if (this->vendor_id_.has_value()) {
        str += fmt::format("  vendor_id:\n"
                           "    unit: \"string\"\n"
                           "    values: \"{}\"\n",
                           this->vendor_id_.value());
    }
    // device name
    if (this->name_.has_value()) {
        str += fmt::format("  name:\n"
                           "    unit: \"string\"\n"
                           "    values: \"{}\"\n",
                           this->name_.value());
    }
    // persistence mode enabled
    if (this->persistence_mode_.has_value()) {
        str += fmt::format("  persistence_mode:\n"
                           "    unit: \"bool\"\n"
                           "    values: {}\n",
                           this->persistence_mode_.value());
    }
    // number of cores
    if (this->num_cores_.has_value()) {
        str += fmt::format("  num_cores:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->num_cores_.value());
    }

    // device compute utilization
    if (this->compute_utilization_.has_value()) {
        str += fmt::format("  compute_utilization:\n"
                           "    unit: \"percentage\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->compute_utilization_.value(), ", "));
    }

    // device memory utilization
    if (this->memory_utilization_.has_value()) {
        str += fmt::format("  memory_utilization:\n"
                           "    unit: \"percentage\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->memory_utilization_.value(), ", "));
    }
    // performance state
    if (this->performance_level_.has_value()) {
        str += fmt::format("  performance_level:\n"
                           "    unit: \"0 - maximum performance; 15 - minimum performance; 32 - unknown\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->performance_level_.value(), ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

std::ostream &operator<<(std::ostream &out, const nvml_general_samples &samples) {
    return out << fmt::format("architecture [string]: {}\n"
                              "byte_order [string]: {}\n"
                              "vendor_id [string]: {}\n"
                              "name [string]: {}\n"
                              "persistence_mode [bool]: {}\n"
                              "num_cores [int]: {}\n"
                              "compute_utilization [%]: [{}]\n"
                              "memory_utilization [%]: [{}]\n"
                              "performance_level [int]: [{}]",
                              detail::value_or_default(samples.get_architecture()),
                              detail::value_or_default(samples.get_byte_order()),
                              detail::value_or_default(samples.get_vendor_id()),
                              detail::value_or_default(samples.get_name()),
                              detail::value_or_default(samples.get_persistence_mode()),
                              detail::value_or_default(samples.get_num_cores()),
                              fmt::join(detail::value_or_default(samples.get_compute_utilization()), ", "),
                              fmt::join(detail::value_or_default(samples.get_memory_utilization()), ", "),
                              fmt::join(detail::value_or_default(samples.get_performance_level()), ", "));
}

//*************************************************************************************************************************************//
//                                                            clock samples                                                            //
//*************************************************************************************************************************************//

std::string nvml_clock_samples::generate_yaml_string() const {
    std::string str{ "clock:\n" };

    // adaptive clock status
    if (this->auto_boosted_clock_enabled_.has_value()) {
        str += fmt::format("  auto_boosted_clock_enabled:\n"
                           "    unit: \"bool\"\n"
                           "    values: {}\n",
                           this->auto_boosted_clock_enabled_.value());
    }
    // minimum graph clock
    if (this->clock_frequency_min_.has_value()) {
        str += fmt::format("  clock_frequency_min:\n"
                           "    unit: \"MHz\"\n"
                           "    values: {}\n",
                           this->clock_frequency_min_.value());
    }
    // maximum graph clock
    if (this->clock_frequency_max_.has_value()) {
        str += fmt::format("  clock_frequency_max:\n"
                           "    unit: \"MHz\"\n"
                           "    values: {}\n",
                           this->clock_frequency_max_.value());
    }
    // minimum memory clock
    if (this->memory_clock_frequency_min_.has_value()) {
        str += fmt::format("  memory_clock_frequency_min:\n"
                           "    unit: \"MHz\"\n"
                           "    values: {}\n",
                           this->memory_clock_frequency_min_.value());
    }
    // maximum memory clock
    if (this->memory_clock_frequency_max_.has_value()) {
        str += fmt::format("  memory_clock_frequency_max:\n"
                           "    unit: \"MHz\"\n"
                           "    values: {}\n",
                           this->memory_clock_frequency_max_.value());
    }
    // maximum SM clock
    if (this->sm_clock_frequency_max_.has_value()) {
        str += fmt::format("  sm_clock_frequency_max:\n"
                           "    unit: \"MHz\"\n"
                           "    values: {}\n",
                           this->sm_clock_frequency_max_.value());
    }
    // the available clock frequencies
    if (this->available_clock_frequencies_.has_value()) {
        str += fmt::format("  available_clock_frequencies:\n"
                           "    unit: \"MHz\"\n"
                           "    values:\n");
        for (const auto &[key, value] : this->available_clock_frequencies_.value()) {
            str += fmt::format("      {}: [{}]\n", key, fmt::join(value, ", "));
        }
    }
    // the available memory clock frequencies
    if (this->available_memory_clock_frequencies_.has_value()) {
        str += fmt::format("  available_memory_clock_frequencies:\n"
                           "    unit: \"MHz\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->available_memory_clock_frequencies_.value(), ", "));
    }

    // graph clock
    if (this->clock_frequency_.has_value()) {
        str += fmt::format("  clock_frequency:\n"
                           "    unit: \"MHz\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->clock_frequency_.value(), ", "));
    }
    // memory clock
    if (this->memory_clock_frequency_.has_value()) {
        str += fmt::format("  memory_clock_frequency:\n"
                           "    unit: \"MHz\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->memory_clock_frequency_.value(), ", "));
    }
    // SM clock
    if (this->sm_clock_frequency_.has_value()) {
        str += fmt::format("  sm_clock_frequency:\n"
                           "    unit: \"MHz\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->sm_clock_frequency_.value(), ", "));
    }
    // clock throttle reason
    if (this->throttle_reason_.has_value()) {
        str += fmt::format("  throttle_reason:\n"
                           "    unit: \"string\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->throttle_reason_.value(), ", "));
    }
    // clock is auto-boosted
    if (this->auto_boosted_clock_.has_value()) {
        str += fmt::format("  auto_boosted_clock:\n"
                           "    unit: \"bool\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->auto_boosted_clock_.value(), ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

std::ostream &operator<<(std::ostream &out, const nvml_clock_samples &samples) {
    return out << fmt::format("auto_boosted_clock_enabled [bool]: {}\n"
                              "clock_frequency_min [MHz]: {}\n"
                              "clock_frequency_max [MHz]: {}\n"
                              "memory_clock_frequency_min [MHz]: {}\n"
                              "memory_clock_frequency_max [MHz]: {}\n"
                              "sm_clock_frequency_max [MHz]: {}\n"
                              "available_clock_frequencies [MHz]: [{}]\n"
                              "available_memory_clock_frequencies [MHz]: [{}]\n"
                              "clock_frequency [MHz]: [{}]\n"
                              "memory_clock_frequency [MHz]: [{}]\n"
                              "sm_clock_frequency [MHz]: [{}]\n"
                              "throttle_reason [string]: [{}]\n"
                              "auto_boosted_clock [bool]: [{}]",
                              detail::value_or_default(samples.get_auto_boosted_clock_enabled()),
                              detail::value_or_default(samples.get_clock_frequency_min()),
                              detail::value_or_default(samples.get_clock_frequency_max()),
                              detail::value_or_default(samples.get_memory_clock_frequency_min()),
                              detail::value_or_default(samples.get_memory_clock_frequency_max()),
                              detail::value_or_default(samples.get_sm_clock_frequency_max()),
                              detail::map_entry_to_string(samples.get_available_clock_frequencies()),
                              fmt::join(detail::value_or_default(samples.get_available_memory_clock_frequencies()), ", "),
                              fmt::join(detail::value_or_default(samples.get_clock_frequency()), ", "),
                              fmt::join(detail::value_or_default(samples.get_memory_clock_frequency()), ", "),
                              fmt::join(detail::value_or_default(samples.get_sm_clock_frequency()), ", "),
                              fmt::join(detail::value_or_default(samples.get_throttle_reason()), ", "),
                              fmt::join(detail::value_or_default(samples.get_auto_boosted_clock()), ", "));
}

//*************************************************************************************************************************************//
//                                                            power samples                                                            //
//*************************************************************************************************************************************//

std::string nvml_power_samples::generate_yaml_string() const {
    std::string str{ "power:\n" };

    // power management limit
    if (this->power_management_limit_.has_value()) {
        str += fmt::format("  power_management_limit:\n"
                           "    unit: \"W\"\n"
                           "    values: {}\n",
                           this->power_management_limit_.value());
    }
    // power enforced limit
    if (this->power_enforced_limit_.has_value()) {
        str += fmt::format("  power_enforced_limit:\n"
                           "    unit: \"W\"\n"
                           "    values: {}\n",
                           this->power_enforced_limit_.value());
    }
    // power measurement type
    if (this->power_measurement_type_.has_value()) {
        str += fmt::format("  power_measurement_type:\n"
                           "    unit: \"string\"\n"
                           "    values: \"{}\"\n",
                           this->power_measurement_type_.value());
    }
    // the power management mode
    if (this->power_management_mode_.has_value()) {
        str += fmt::format("  power_management_mode:\n"
                           "    unit: \"bool\"\n"
                           "    values: {}\n",
                           this->power_management_mode_.value());
    }
    // available power levels
    if (this->available_power_profiles_.has_value()) {
        str += fmt::format("  available_power_profiles:\n"
                           "    unit: \"int\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->available_power_profiles_.value(), ", "));
    }

    // current power usage
    if (this->power_usage_.has_value()) {
        str += fmt::format("  power_usage:\n"
                           "    unit: \"W\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->power_usage_.value(), ", "));
    }
    // total energy consumed
    if (this->power_total_energy_consumption_.has_value()) {
        str += fmt::format("  power_total_energy_consumed:\n"
                           "    unit: \"J\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->power_total_energy_consumption_.value(), ", "));
    }
    // power state
    if (this->power_profile_.has_value()) {
        str += fmt::format("  power_profile:\n"
                           "    unit: \"int\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->power_profile_.value(), ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

std::ostream &operator<<(std::ostream &out, const nvml_power_samples &samples) {
    return out << fmt::format("power_management_limit [W]: {}\n"
                              "power_enforced_limit [W]: {}\n"
                              "power_measurement_type [string]: {}\n"
                              "power_management_mode [bool]: {}\n"
                              "available_power_profiles [int]: [{}]\n"
                              "power_usage [W]: [{}]\n"
                              "power_total_energy_consumption [J]: [{}]"
                              "power_profile [int]: [{}]\n",
                              detail::value_or_default(samples.get_power_management_limit()),
                              detail::value_or_default(samples.get_power_enforced_limit()),
                              detail::value_or_default(samples.get_power_measurement_type()),
                              detail::value_or_default(samples.get_power_management_mode()),
                              fmt::join(detail::value_or_default(samples.get_available_power_profiles()), ", "),
                              fmt::join(detail::value_or_default(samples.get_power_usage()), ", "),
                              fmt::join(detail::value_or_default(samples.get_power_total_energy_consumption()), ", "),
                              fmt::join(detail::value_or_default(samples.get_power_profile()), ", "));
}

//*************************************************************************************************************************************//
//                                                            memory samples                                                           //
//*************************************************************************************************************************************//

std::string nvml_memory_samples::generate_yaml_string() const {
    std::string str{ "memory:\n" };

    // total memory size
    if (this->memory_total_.has_value()) {
        str += fmt::format("  memory_total:\n"
                           "    unit: \"B\"\n"
                           "    values: {}\n",
                           this->memory_total_.value());
    }
    // maximum PCIe link speed
    if (this->pcie_link_max_speed_.has_value()) {
        str += fmt::format("  pcie_max_bandwidth:\n"
                           "    unit: \"MBPS\"\n"
                           "    values: {}\n",
                           this->pcie_link_max_speed_.value());
    }
    // memory bus width
    if (this->memory_bus_width_.has_value()) {
        str += fmt::format("  memory_bus_width:\n"
                           "    unit: \"Bit\"\n"
                           "    values: {}\n",
                           this->memory_bus_width_.value());
    }
    // maximum PCIe link generation
    if (this->max_pcie_link_generation_.has_value()) {
        str += fmt::format("  max_pcie_link_generation:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->max_pcie_link_generation_.value());
    }

    // free memory size
    if (this->memory_free_.has_value()) {
        str += fmt::format("  memory_free:\n"
                           "    unit: \"B\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->memory_free_.value(), ", "));
    }
    // used memory size
    if (this->memory_used_.has_value()) {
        str += fmt::format("  memory_used:\n"
                           "    unit: \"B\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->memory_used_.value(), ", "));
    }
    // PCIe link speed
    if (this->pcie_link_speed_.has_value()) {
        str += fmt::format("  pcie_bandwidth:\n"
                           "    unit: \"MBPS\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->pcie_link_speed_.value(), ", "));
    }
    // PCIe link width
    if (this->pcie_link_width_.has_value()) {
        str += fmt::format("  pcie_link_width:\n"
                           "    unit: \"int\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->pcie_link_width_.value(), ", "));
    }
    // PCIe link generation
    if (this->pcie_link_generation_.has_value()) {
        str += fmt::format("  pcie_link_generation:\n"
                           "    unit: \"int\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->pcie_link_generation_.value(), ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

std::ostream &operator<<(std::ostream &out, const nvml_memory_samples &samples) {
    return out << fmt::format("memory_total [B]: {}\n"
                              "pcie_link_max_speed [MBPS]: {}\n"
                              "memory_bus_width [Bit]: {}\n"
                              "max_pcie_link_generation [int]: {}\n"
                              "memory_free [B]: [{}]\n"
                              "memory_used [B]: [{}]\n"
                              "pcie_link_speed [MBPS]: [{}]\n"
                              "pcie_link_width [int]: [{}]\n"
                              "pcie_link_generation [int]: [{}]",
                              detail::value_or_default(samples.get_memory_total()),
                              detail::value_or_default(samples.get_pcie_link_max_speed()),
                              detail::value_or_default(samples.get_memory_bus_width()),
                              detail::value_or_default(samples.get_max_pcie_link_generation()),
                              fmt::join(detail::value_or_default(samples.get_memory_free()), ", "),
                              fmt::join(detail::value_or_default(samples.get_memory_used()), ", "),
                              fmt::join(detail::value_or_default(samples.get_pcie_link_speed()), ", "),
                              fmt::join(detail::value_or_default(samples.get_pcie_link_width()), ", "),
                              fmt::join(detail::value_or_default(samples.get_pcie_link_generation()), ", "));
}

//*************************************************************************************************************************************//
//                                                         temperature samples                                                         //
//*************************************************************************************************************************************//

std::string nvml_temperature_samples::generate_yaml_string() const {
    std::string str{ "temperature:\n" };

    // number of fans
    if (this->num_fans_.has_value()) {
        str += fmt::format("  num_fans:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->num_fans_.value());
    }
    // min fan speed
    if (this->min_fan_speed_.has_value()) {
        str += fmt::format("  min_fan_speed:\n"
                           "    unit: \"percentage\"\n"
                           "    values: {}\n",
                           this->min_fan_speed_.value());
    }
    // max fan speed
    if (this->max_fan_speed_.has_value()) {
        str += fmt::format("  max_fan_speed:\n"
                           "    unit: \"percentage\"\n"
                           "    values: {}\n",
                           this->max_fan_speed_.value());
    }
    // temperature threshold GPU max
    if (this->temperature_threshold_gpu_max_.has_value()) {
        str += fmt::format("  temperature_gpu_max:\n"
                           "    unit: \"°C\"\n"
                           "    values: {}\n",
                           this->temperature_threshold_gpu_max_.value());
    }
    // temperature threshold memory max
    if (this->temperature_threshold_mem_max_.has_value()) {
        str += fmt::format("  temperature_mem_max:\n"
                           "    unit: \"°C\"\n"
                           "    values: {}\n",
                           this->temperature_threshold_mem_max_.value());
    }

    // fan speed
    if (this->fan_speed_.has_value()) {
        str += fmt::format("  fan_speed:\n"
                           "    unit: \"percentage\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->fan_speed_.value(), ", "));
    }
    // temperature GPU
    if (this->temperature_gpu_.has_value()) {
        str += fmt::format("  temperature_gpu:\n"
                           "    unit: \"°C\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->temperature_gpu_.value(), ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

std::ostream &operator<<(std::ostream &out, const nvml_temperature_samples &samples) {
    return out << fmt::format("num_fans [int]: {}\n"
                              "min_fan_speed [%]: {}\n"
                              "max_fan_speed [%]: {}\n"
                              "temperature_threshold_gpu_max [°C]: {}\n"
                              "temperature_threshold_mem_max [°C]: {}\n"
                              "fan_speed [%]: [{}]\n"
                              "temperature_gpu [°C]: [{}]",
                              detail::value_or_default(samples.get_num_fans()),
                              detail::value_or_default(samples.get_min_fan_speed()),
                              detail::value_or_default(samples.get_max_fan_speed()),
                              detail::value_or_default(samples.get_temperature_threshold_gpu_max()),
                              detail::value_or_default(samples.get_temperature_threshold_mem_max()),
                              fmt::join(detail::value_or_default(samples.get_fan_speed()), ", "),
                              fmt::join(detail::value_or_default(samples.get_temperature_gpu()), ", "));
}

}  // namespace hws
