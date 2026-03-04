/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hws/gpu_nvidia/nvml_samples.hpp"

#include "hws/utility.hpp"  // hws::detail::{value_or_default, map_entry_to_string, quote}

#include "fmt/format.h"  // fmt::format
#include "fmt/ranges.h"  // fmt::join

#include <ostream>  // std::ostream
#include <string>   // std::string

namespace hws {

//*************************************************************************************************************************************//
//                                                           general samples                                                           //
//*************************************************************************************************************************************//

bool nvml_general_samples::has_samples() const {
    return this->architecture_.has_value() || this->byte_order_.has_value() || this->vendor_id_.has_value() || this->name_.has_value()
           || this->persistence_mode_.has_value() || this->num_cores_.has_value() || this->compute_utilization_.has_value()
           || this->memory_utilization_.has_value() || this->performance_level_.has_value();
}

std::string nvml_general_samples::generate_yaml_string() const {
    // if no samples are available, return an empty string
    if (!this->has_samples()) {
        return "";
    }

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

    return str;
}

std::ostream &operator<<(std::ostream &out, const nvml_general_samples &samples) {
    return out << fmt::format("architecture [string]: {}\n"
                              "byte_order [string]: {}\n"
                              "num_cores [int]: {}\n"
                              "vendor_id [string]: {}\n"
                              "name [string]: {}\n"
                              "persistence_mode [bool]: {}\n"
                              "compute_utilization [%]: [{}]\n"
                              "memory_utilization [%]: [{}]\n"
                              "performance_level [int]: [{}]",
                              detail::value_or_default(samples.get_architecture()),
                              detail::value_or_default(samples.get_byte_order()),
                              detail::value_or_default(samples.get_num_cores()),
                              detail::value_or_default(samples.get_vendor_id()),
                              detail::value_or_default(samples.get_name()),
                              detail::value_or_default(samples.get_persistence_mode()),
                              fmt::join(detail::value_or_default(samples.get_compute_utilization()), ", "),
                              fmt::join(detail::value_or_default(samples.get_memory_utilization()), ", "),
                              fmt::join(detail::value_or_default(samples.get_performance_level()), ", "));
}

//*************************************************************************************************************************************//
//                                                            clock samples                                                            //
//*************************************************************************************************************************************//

bool nvml_clock_samples::has_samples() const {
    return this->auto_boosted_clock_enabled_.has_value() || this->clock_frequency_min_.has_value() || this->clock_frequency_max_.has_value()
           || this->memory_clock_frequency_min_.has_value() || this->memory_clock_frequency_max_.has_value() || this->sm_clock_frequency_max_.has_value()
           || this->available_clock_frequencies_.has_value() || this->available_memory_clock_frequencies_.has_value() || this->clock_frequency_.has_value()
           || this->memory_clock_frequency_.has_value() || this->sm_clock_frequency_.has_value() || this->throttle_reason_.has_value()
           || this->throttle_reason_string_.has_value() || this->auto_boosted_clock_.has_value();
}

std::string nvml_clock_samples::generate_yaml_string() const {
    // if no samples are available, return an empty string
    if (!this->has_samples()) {
        return "";
    }

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
            str += fmt::format("      memory_clock_frequency_{}: [{}]\n", key, fmt::join(value, ", "));
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
    // clock throttle reason as bitmask
    if (this->throttle_reason_.has_value()) {
        str += fmt::format("  throttle_reason:\n"
                           "    unit: \"bitmask\"\n"
                           "    values: [{}]\n",
                           fmt::join(detail::quote(this->throttle_reason_.value()), ", "));
    }
    // clock throttle reason as string
    if (this->throttle_reason_string_.has_value()) {
        str += fmt::format("  throttle_reason_string:\n"
                           "    unit: \"string\"\n"
                           "    values: [{}]\n",
                           fmt::join(detail::quote(this->throttle_reason_string_.value()), ", "));
    }
    // clock is auto-boosted
    if (this->auto_boosted_clock_.has_value()) {
        str += fmt::format("  auto_boosted_clock:\n"
                           "    unit: \"bool\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->auto_boosted_clock_.value(), ", "));
    }

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
                              "throttle_reason [bitmask]: [{}]\n"
                              "throttle_reason_string [string]: [{}]\n"
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
                              fmt::join(detail::value_or_default(samples.get_throttle_reason_string()), ", "),
                              fmt::join(detail::value_or_default(samples.get_auto_boosted_clock()), ", "));
}

//*************************************************************************************************************************************//
//                                                            power samples                                                            //
//*************************************************************************************************************************************//

bool nvml_power_samples::has_samples() const {
    return this->power_management_limit_.has_value() || this->power_enforced_limit_.has_value() || this->power_measurement_type_.has_value()
           || this->power_management_mode_.has_value() || this->available_power_profiles_.has_value() || this->power_usage_.has_value()
           || this->power_total_energy_consumption_.has_value() || this->power_profile_.has_value() || this->system_power_usage_.has_value();
}

std::string nvml_power_samples::generate_yaml_string() const {
    // if no samples are available, return an empty string
    if (!this->has_samples()) {
        return "";
    }

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
    // current system power usage
    if (this->system_power_usage_.has_value()) {
        str += fmt::format("  system_power_usage:\n"
                           "    unit: \"W\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->system_power_usage_.value(), ", "));
    }

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
                              "power_profile [int]: [{}]\n"
                              "system_power_usage [W]: [{}]\n",
                              detail::value_or_default(samples.get_power_management_limit()),
                              detail::value_or_default(samples.get_power_enforced_limit()),
                              detail::value_or_default(samples.get_power_measurement_type()),
                              detail::value_or_default(samples.get_power_management_mode()),
                              fmt::join(detail::value_or_default(samples.get_available_power_profiles()), ", "),
                              fmt::join(detail::value_or_default(samples.get_power_usage()), ", "),
                              fmt::join(detail::value_or_default(samples.get_power_total_energy_consumption()), ", "),
                              fmt::join(detail::value_or_default(samples.get_power_profile()), ", "),
                              fmt::join(detail::value_or_default(samples.get_system_power_usage()), ", "));
}

//*************************************************************************************************************************************//
//                                                            memory samples                                                           //
//*************************************************************************************************************************************//

bool nvml_memory_samples::has_samples() const {
    return this->memory_total_.has_value() || this->pcie_link_speed_max_.has_value() || this->pcie_link_generation_max_.has_value()
           || this->num_pcie_lanes_max_.has_value() || this->memory_bus_width_.has_value() || this->memory_used_.has_value()
           || this->memory_free_.has_value() || this->num_pcie_lanes_.has_value() || this->pcie_link_generation_.has_value()
           || this->pcie_link_speed_.has_value();
}

std::string nvml_memory_samples::generate_yaml_string() const {
    // if no samples are available, return an empty string
    if (!this->has_samples()) {
        return "";
    }

    std::string str{ "memory:\n" };

    // total memory size
    if (this->memory_total_.has_value()) {
        str += fmt::format("  memory_total:\n"
                           "    unit: \"B\"\n"
                           "    values: {}\n",
                           this->memory_total_.value());
    }
    // maximum PCIe link speed
    if (this->pcie_link_speed_max_.has_value()) {
        str += fmt::format("  pcie_link_speed_max:\n"
                           "    unit: \"MBPS\"\n"
                           "    values: {}\n",
                           this->pcie_link_speed_max_.value());
    }
    // maximum PCIe link generation
    if (this->pcie_link_generation_max_.has_value()) {
        str += fmt::format("  pcie_link_generation_max:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->pcie_link_generation_max_.value());
    }
    // maximum number of available PCIe lanes
    if (this->num_pcie_lanes_max_.has_value()) {
        str += fmt::format("  num_pcie_lanes_max:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->num_pcie_lanes_max_.value());
    }
    // memory bus width
    if (this->memory_bus_width_.has_value()) {
        str += fmt::format("  memory_bus_width:\n"
                           "    unit: \"Bit\"\n"
                           "    values: {}\n",
                           this->memory_bus_width_.value());
    }

    // used memory size
    if (this->memory_used_.has_value()) {
        str += fmt::format("  memory_used:\n"
                           "    unit: \"B\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->memory_used_.value(), ", "));
    }
    // free memory size
    if (this->memory_free_.has_value()) {
        str += fmt::format("  memory_free:\n"
                           "    unit: \"B\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->memory_free_.value(), ", "));
    }
    // PCIe link width
    if (this->num_pcie_lanes_.has_value()) {
        str += fmt::format("  num_pcie_lanes:\n"
                           "    unit: \"int\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->num_pcie_lanes_.value(), ", "));
    }
    // PCIe link generation
    if (this->pcie_link_generation_.has_value()) {
        str += fmt::format("  pcie_link_generation:\n"
                           "    unit: \"int\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->pcie_link_generation_.value(), ", "));
    }
    // PCIe link speed
    if (this->pcie_link_speed_.has_value()) {
        str += fmt::format("  pcie_link_speed:\n"
                           "    unit: \"MBPS\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->pcie_link_speed_.value(), ", "));
    }

    return str;
}

std::ostream &operator<<(std::ostream &out, const nvml_memory_samples &samples) {
    return out << fmt::format("memory_total [B]: {}\n"
                              "pcie_link_speed_max [MBPS]: {}\n"
                              "pcie_link_generation_max [int]: {}\n"
                              "num_pcie_lanes_max [int]: {}\n"
                              "memory_bus_width [Bit]: {}\n"
                              "memory_used [B]: [{}]\n"
                              "memory_free [B]: [{}]\n"
                              "num_pcie_lanes [int]: [{}]\n"
                              "pcie_link_generation [int]: [{}]\n"
                              "pcie_link_speed [MBPS]: [{}]",
                              detail::value_or_default(samples.get_memory_total()),
                              detail::value_or_default(samples.get_pcie_link_speed_max()),
                              detail::value_or_default(samples.get_pcie_link_generation_max()),
                              detail::value_or_default(samples.get_num_pcie_lanes_max()),
                              detail::value_or_default(samples.get_memory_bus_width()),
                              fmt::join(detail::value_or_default(samples.get_memory_used()), ", "),
                              fmt::join(detail::value_or_default(samples.get_memory_free()), ", "),
                              fmt::join(detail::value_or_default(samples.get_num_pcie_lanes()), ", "),
                              fmt::join(detail::value_or_default(samples.get_pcie_link_generation()), ", "),
                              fmt::join(detail::value_or_default(samples.get_pcie_link_speed()), ", "));
}

//*************************************************************************************************************************************//
//                                                         temperature samples                                                         //
//*************************************************************************************************************************************//

bool nvml_temperature_samples::has_samples() const {
    return this->num_fans_.has_value() || this->fan_speed_min_.has_value() || this->fan_speed_max_.has_value() || this->temperature_max_.has_value()
           || this->memory_temperature_max_.has_value() || this->fan_speed_percentage_.has_value() || this->temperature_.has_value();
}

std::string nvml_temperature_samples::generate_yaml_string() const {
    // if no samples are available, return an empty string
    if (!this->has_samples()) {
        return "";
    }

    std::string str{ "temperature:\n" };

    // number of fans
    if (this->num_fans_.has_value()) {
        str += fmt::format("  num_fans:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->num_fans_.value());
    }
    // min fan speed
    if (this->fan_speed_min_.has_value()) {
        str += fmt::format("  fan_speed_min:\n"
                           "    unit: \"percentage\"\n"
                           "    values: {}\n",
                           this->fan_speed_min_.value());
    }
    // max fan speed
    if (this->fan_speed_max_.has_value()) {
        str += fmt::format("  fan_speed_max:\n"
                           "    unit: \"percentage\"\n"
                           "    values: {}\n",
                           this->fan_speed_max_.value());
    }
    // temperature threshold GPU max
    if (this->temperature_max_.has_value()) {
        str += fmt::format("  temperature_max:\n"
                           "    unit: \"°C\"\n"
                           "    values: {}\n",
                           this->temperature_max_.value());
    }
    // temperature threshold memory max
    if (this->memory_temperature_max_.has_value()) {
        str += fmt::format("  memory_temperature_max:\n"
                           "    unit: \"°C\"\n"
                           "    values: {}\n",
                           this->memory_temperature_max_.value());
    }

    // fan speed
    if (this->fan_speed_percentage_.has_value()) {
        str += fmt::format("  fan_speed_percentage:\n"
                           "    unit: \"percentage\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->fan_speed_percentage_.value(), ", "));
    }
    // temperature GPU
    if (this->temperature_.has_value()) {
        str += fmt::format("  temperature:\n"
                           "    unit: \"°C\"\n"
                           "    values: [{}]\n",
                           fmt::join(this->temperature_.value(), ", "));
    }

    return str;
}

std::ostream &operator<<(std::ostream &out, const nvml_temperature_samples &samples) {
    return out << fmt::format("num_fans [int]: {}\n"
                              "min_fan_speed [%]: {}\n"
                              "max_fan_speed [%]: {}\n"
                              "temperature__max [°C]: {}\n"
                              "memory_temperature_max [°C]: {}\n"
                              "fan_speed_percentage [%]: [{}]\n"
                              "temperature [°C]: [{}]",
                              detail::value_or_default(samples.get_num_fans()),
                              detail::value_or_default(samples.get_fan_speed_min()),
                              detail::value_or_default(samples.get_fan_speed_max()),
                              detail::value_or_default(samples.get_temperature_max()),
                              detail::value_or_default(samples.get_memory_temperature_max()),
                              fmt::join(detail::value_or_default(samples.get_fan_speed_percentage()), ", "),
                              fmt::join(detail::value_or_default(samples.get_temperature()), ", "));
}

}  // namespace hws
