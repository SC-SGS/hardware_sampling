/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hardware_sampling/gpu_nvidia/nvml_samples.hpp"

#include "hardware_sampling/utility.hpp"  // hws::detail::{value_or_default, map_entry_to_string, add_yaml_entry}

#include "fmt/format.h"  // fmt::format
#include "fmt/ranges.h"  // fmt::join
#include "ryml.hpp"      // ryml::NodeRef, ryml::MAP

#include <ostream>  // std::ostream
#include <string>   // std::string

namespace hws {

//*************************************************************************************************************************************//
//                                                           general samples                                                           //
//*************************************************************************************************************************************//

void nvml_general_samples::add_yaml_entries(ryml::NodeRef &root) const {
    ryml::NodeRef general_samples = root["general"];
    general_samples |= ryml::MAP;

    // device architecture
    detail::add_yaml_entry(general_samples, "architecture", "string", this->architecture_);
    // device byte order
    detail::add_yaml_entry(general_samples, "byte_order", "string", this->byte_order_);
    // the vendor specific ID
    detail::add_yaml_entry(general_samples, "vendor_id", "string", this->vendor_id_);
    // device name
    detail::add_yaml_entry(general_samples, "name", "string", this->name_);
    // persistence mode enabled
    detail::add_yaml_entry(general_samples, "persistence_mode", "bool", this->persistence_mode_);
    // number of cores
    detail::add_yaml_entry(general_samples, "num_cores", "int", this->num_cores_);

    // device compute utilization
    detail::add_yaml_entry(general_samples, "compute_utilization", "percentage", this->compute_utilization_);
    // device memory utilization
    detail::add_yaml_entry(general_samples, "memory_utilization", "percentage", this->memory_utilization_);
    // performance state
    detail::add_yaml_entry(general_samples, "performance_level", "0 - maximum performance; 15 - minimum performance; 32 - unknown", this->performance_level_);
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

void nvml_clock_samples::add_yaml_entries(ryml::NodeRef &root) const {
    ryml::NodeRef clock_samples = root["clock"];
    clock_samples |= ryml::MAP;

    // adaptive clock status
    detail::add_yaml_entry(clock_samples, "auto_boosted_clock_enabled", "bool", this->auto_boosted_clock_enabled_);
    // minimum graph clock
    detail::add_yaml_entry(clock_samples, "clock_frequency_min", "MHz", this->clock_frequency_min_);
    // maximum graph clock
    detail::add_yaml_entry(clock_samples, "clock_frequency_max", "MHz", this->clock_frequency_max_);
    // minimum memory clock
    detail::add_yaml_entry(clock_samples, "memory_clock_frequency_min", "MHz", this->memory_clock_frequency_min_);
    // maximum memory clock
    detail::add_yaml_entry(clock_samples, "memory_clock_frequency_max", "MHz", this->memory_clock_frequency_max_);
    // maximum SM clock
    detail::add_yaml_entry(clock_samples, "sm_clock_frequency_max", "MHz", this->sm_clock_frequency_max_);
    // the available clock frequencies // TODO:
//    detail::add_yaml_entry(clock_samples, "available_clock_frequencies", "MHz", this->available_clock_frequencies_);
//    if (this->available_clock_frequencies_.has_value()) {
//        str += fmt::format("  available_clock_frequencies:\n"
//                           "    unit: \"MHz\"\n"
//                           "    values:\n");
//        for (const auto &[key, value] : this->available_clock_frequencies_.value()) {
//            str += fmt::format("      memory_clock_frequency_{}: [{}]\n", key, fmt::join(value, ", "));
//        }
//    }
    // the available memory clock frequencies
    detail::add_yaml_entry(clock_samples, "available_memory_clock_frequencies", "MHz", this->available_memory_clock_frequencies_);

    // graph clock
    detail::add_yaml_entry(clock_samples, "clock_frequency", "MHz", this->clock_frequency_);
    // memory clock
    detail::add_yaml_entry(clock_samples, "memory_clock_frequency", "MHz", this->memory_clock_frequency_);
    // SM clock
    detail::add_yaml_entry(clock_samples, "sm_clock_frequency", "MHz", this->sm_clock_frequency_);
    // clock throttle reason
    detail::add_yaml_entry(clock_samples, "throttle_reason", "string", this->throttle_reason_);
    // clock is auto-boosted
    detail::add_yaml_entry(clock_samples, "auto_boosted_clock", "bool", this->auto_boosted_clock_);  // TODO: output
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

void nvml_power_samples::add_yaml_entries(ryml::NodeRef &root) const {
    ryml::NodeRef power_samples = root["power"];
    power_samples |= ryml::MAP;

    // power management limit
    detail::add_yaml_entry(power_samples, "power_management_limit", "W", this->power_management_limit_);
    // power enforced limit
    detail::add_yaml_entry(power_samples, "power_enforced_limit", "W", this->power_enforced_limit_);
    // power measurement type
    detail::add_yaml_entry(power_samples, "power_measurement_type", "string", this->power_measurement_type_);
    // the power management mode
    detail::add_yaml_entry(power_samples, "power_management_mode", "bool", this->power_management_mode_);
    // available power levels
    detail::add_yaml_entry(power_samples, "available_power_profiles", "int", this->available_power_profiles_);

    // current power usage
    detail::add_yaml_entry(power_samples, "power_usage", "W", this->power_usage_);
    // total energy consumed
    detail::add_yaml_entry(power_samples, "power_total_energy_consumed", "J", this->power_total_energy_consumption_);
    // power state
    detail::add_yaml_entry(power_samples, "power_profile", "int", this->power_profile_);
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

void nvml_memory_samples::add_yaml_entries(ryml::NodeRef &root) const {
    ryml::NodeRef memory_samples = root["memory"];
    memory_samples |= ryml::MAP;

    // total memory size
    detail::add_yaml_entry(memory_samples, "memory_total", "B", this->memory_total_);
    // maximum PCIe link speed
    detail::add_yaml_entry(memory_samples, "pcie_link_speed_max", "MBPS", this->pcie_link_speed_max_);
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

    // remove last newline
    str.pop_back();

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

    // remove last newline
    str.pop_back();

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
