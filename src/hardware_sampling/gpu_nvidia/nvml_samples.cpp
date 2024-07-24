/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hardware_sampling/gpu_nvidia/nvml_samples.hpp"

#include "hardware_sampling/utility.hpp"  // hws::detail::{value_or_default, join}

#include "nvml.h"  // NVML_ADAPTIVE_CLOCKING_INFO_STATUS_ENABLED

#include <format>   // std::format
#include <ostream>  // std::ostream
#include <string>   // std::string

namespace hws {

//*************************************************************************************************************************************//
//                                                           general samples                                                           //
//*************************************************************************************************************************************//

std::string nvml_general_samples::generate_yaml_string() const {
    std::string str{ "general:\n" };

    // device name
    if (this->name_.has_value()) {
        str += std::format("  name:\n"
                           "    unit: \"string\"\n"
                           "    values: \"{}\"\n",
                           this->name_.value());
    }
    // persistence mode enabled
    if (this->persistence_mode_.has_value()) {
        str += std::format("  persistence_mode:\n"
                           "    unit: \"bool\"\n"
                           "    values: {}\n",
                           this->persistence_mode_.value());
    }
    // number of cores
    if (this->num_cores_.has_value()) {
        str += std::format("  num_cores:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->num_cores_.value());
    }

    // performance state
    if (this->performance_state_.has_value()) {
        str += std::format("  performance_state:\n"
                           "    unit: \"0 - maximum performance; 15 - minimum performance; 32 - unknown\"\n"
                           "    values: [{}]\n",
                           detail::join(this->performance_state_.value(), ", "));
    }
    // device compute utilization
    if (this->utilization_gpu_.has_value()) {
        str += std::format("  utilization_gpu:\n"
                           "    unit: \"percentage\"\n"
                           "    values: [{}]\n",
                           detail::join(this->utilization_gpu_.value(), ", "));
    }

    // device compute utilization
    if (this->utilization_mem_.has_value()) {
        str += std::format("  utilization_mem:\n"
                           "    unit: \"percentage\"\n"
                           "    values: [{}]\n",
                           detail::join(this->utilization_mem_.value(), ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

std::ostream &operator<<(std::ostream &out, const nvml_general_samples &samples) {
    return out << std::format("name [string]: {}\n"
                              "persistence_mode [bool]: {}\n"
                              "num_cores [int]: {}\n"
                              "performance_state [int]: [{}]\n"
                              "utilization_gpu [%]: [{}]\n"
                              "utilization_mem [%]: [{}]",
                              detail::value_or_default(samples.get_name()),
                              detail::value_or_default(samples.get_persistence_mode()),
                              detail::value_or_default(samples.get_num_cores()),
                              detail::join(detail::value_or_default(samples.get_performance_state()), ", "),
                              detail::join(detail::value_or_default(samples.get_utilization_gpu()), ", "),
                              detail::join(detail::value_or_default(samples.get_utilization_mem()), ", "));
}

//*************************************************************************************************************************************//
//                                                            clock samples                                                            //
//*************************************************************************************************************************************//

std::string nvml_clock_samples::generate_yaml_string() const {
    std::string str{ "clock:\n" };

    // adaptive clock status
    if (this->adaptive_clock_status_.has_value()) {
        str += std::format("  adaptive_clock_status:\n"
                           "    unit: \"bool\"\n"
                           "    values: {}\n",
                           this->adaptive_clock_status_.value() == NVML_ADAPTIVE_CLOCKING_INFO_STATUS_ENABLED);
    }
    // maximum SM clock
    if (this->clock_sm_max_.has_value()) {
        str += std::format("  clock_sm_max:\n"
                           "    unit: \"MHz\"\n"
                           "    values: {}\n",
                           this->clock_sm_max_.value());
    }
    // minimum memory clock
    if (this->clock_mem_min_.has_value()) {
        str += std::format("  clock_mem_min:\n"
                           "    unit: \"MHz\"\n"
                           "    values: {}\n",
                           this->clock_mem_min_.value());
    }
    // maximum memory clock
    if (this->clock_mem_max_.has_value()) {
        str += std::format("  clock_mem_max:\n"
                           "    unit: \"MHz\"\n"
                           "    values: {}\n",
                           this->clock_mem_max_.value());
    }
    // minimum graph clock
    if (this->clock_graph_min_.has_value()) {
        str += std::format("  clock_gpu_min:\n"
                           "    unit: \"MHz\"\n"
                           "    values: {}\n",
                           this->clock_graph_min_.value());
    }
    // maximum graph clock
    if (this->clock_graph_max_.has_value()) {
        str += std::format("  clock_gpu_max:\n"
                           "    unit: \"MHz\"\n"
                           "    values: {}\n",
                           this->clock_graph_max_.value());
    }

    // SM clock
    if (this->clock_sm_.has_value()) {
        str += std::format("  clock_sm:\n"
                           "    unit: \"MHz\"\n"
                           "    values: [{}]\n",
                           detail::join(this->clock_sm_.value(), ", "));
    }
    // memory clock
    if (this->clock_mem_.has_value()) {
        str += std::format("  clock_mem:\n"
                           "    unit: \"MHz\"\n"
                           "    values: [{}]\n",
                           detail::join(this->clock_mem_.value(), ", "));
    }
    // graph clock
    if (this->clock_graph_.has_value()) {
        str += std::format("  clock_gpu:\n"
                           "    unit: \"MHz\"\n"
                           "    values: [{}]\n",
                           detail::join(this->clock_graph_.value(), ", "));
    }
    // clock throttle reason
    if (this->clock_throttle_reason_.has_value()) {
        str += std::format("  clock_throttle_reason:\n"
                           "    unit: \"bitmask\"\n"
                           "    values: [{}]\n",
                           detail::join(this->clock_throttle_reason_.value(), ", "));
    }
    // clock is auto-boosted
    if (this->auto_boosted_clocks_.has_value()) {
        str += std::format("  auto_boosted_clocks:\n"
                           "    unit: \"bool\"\n"
                           "    values: [{}]\n",
                           detail::join(this->auto_boosted_clocks_.value(), ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

std::ostream &operator<<(std::ostream &out, const nvml_clock_samples &samples) {
    return out << std::format("adaptive_clock_status [int]: {}\n"
                              "clock_graph_min [MHz]: {}\n"
                              "clock_graph_max [MHz]: {}\n"
                              "clock_sm_max [MHz]: {}\n"
                              "clock_mem_min [MHz]: {}\n"
                              "clock_mem_max [MHz]: {}\n"
                              "clock_graph [MHz]: [{}]\n"
                              "clock_sm [MHz]: [{}]\n"
                              "clock_mem [MHz]: [{}]\n"
                              "clock_throttle_reason [bitmask]: [{}]\n"
                              "auto_boosted_clocks [bool]: [{}]",
                              detail::value_or_default(samples.get_adaptive_clock_status()),
                              detail::value_or_default(samples.get_clock_graph_min()),
                              detail::value_or_default(samples.get_clock_graph_max()),
                              detail::value_or_default(samples.get_clock_sm_max()),
                              detail::value_or_default(samples.get_clock_mem_min()),
                              detail::value_or_default(samples.get_clock_mem_max()),
                              detail::join(detail::value_or_default(samples.get_clock_graph()), ", "),
                              detail::join(detail::value_or_default(samples.get_clock_sm()), ", "),
                              detail::join(detail::value_or_default(samples.get_clock_mem()), ", "),
                              detail::join(detail::value_or_default(samples.get_clock_throttle_reason()), ", "),
                              detail::join(detail::value_or_default(samples.get_auto_boosted_clocks()), ", "));
}

//*************************************************************************************************************************************//
//                                                            power samples                                                            //
//*************************************************************************************************************************************//

std::string nvml_power_samples::generate_yaml_string() const {
    std::string str{ "power:\n" };

    // the power management mode
    if (this->power_management_mode_.has_value()) {
        str += std::format("  power_management_mode:\n"
                           "    unit: \"bool\"\n"
                           "    values: {}\n",
                           this->power_management_mode_.value());
    }
    // power management limit
    if (this->power_management_limit_.has_value()) {
        str += std::format("  power_management_limit:\n"
                           "    unit: \"mW\"\n"
                           "    values: {}\n",
                           this->power_management_limit_.value());
    }
    // power enforced limit
    if (this->power_enforced_limit_.has_value()) {
        str += std::format("  power_enforced_limit:\n"
                           "    unit: \"mW\"\n"
                           "    values: {}\n",
                           this->power_enforced_limit_.value());
    }

    // power state
    if (this->power_state_.has_value()) {
        str += std::format("  power_state:\n"
                           "    unit: \"0 - maximum performance; 15 - minimum performance; 32 - unknown\"\n"
                           "    values: [{}]\n",
                           detail::join(this->power_state_.value(), ", "));
    }
    // current power usage
    if (this->power_usage_.has_value()) {
        str += std::format("  power_usage:\n"
                           "    unit: \"mW\"\n"
                           "    values: [{}]\n",
                           detail::join(this->power_usage_.value(), ", "));
    }
    // total energy consumed
    if (this->power_total_energy_consumption_.has_value()) {
        decltype(nvml_power_samples::power_total_energy_consumption_)::value_type consumed_energy(this->power_total_energy_consumption_->size());
        for (std::size_t i = 0; i < consumed_energy.size(); ++i) {
            consumed_energy[i] = this->power_total_energy_consumption_.value()[i] - this->power_total_energy_consumption_->front();
        }
        str += std::format("  power_total_energy_consumed:\n"
                           "    unit: \"J\"\n"
                           "    values: [{}]\n",
                           detail::join(consumed_energy, ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

std::ostream &operator<<(std::ostream &out, const nvml_power_samples &samples) {
    return out << std::format("power_management_mode [bool]: {}\n"
                              "power_management_limit [mW]: {}\n"
                              "power_enforced_limit [mW]: {}\n"
                              "power_state [int]: [{}]\n"
                              "power_usage [mW]: [{}]\n"
                              "power_total_energy_consumption [J]: [{}]",
                              detail::value_or_default(samples.get_power_management_mode()),
                              detail::value_or_default(samples.get_power_management_limit()),
                              detail::value_or_default(samples.get_power_enforced_limit()),
                              detail::join(detail::value_or_default(samples.get_power_state()), ", "),
                              detail::join(detail::value_or_default(samples.get_power_usage()), ", "),
                              detail::join(detail::value_or_default(samples.get_power_total_energy_consumption()), ", "));
}

//*************************************************************************************************************************************//
//                                                            memory samples                                                           //
//*************************************************************************************************************************************//

std::string nvml_memory_samples::generate_yaml_string() const {
    std::string str{ "memory:\n" };

    // total memory size
    if (this->memory_total_.has_value()) {
        str += std::format("  memory_total:\n"
                           "    unit: \"B\"\n"
                           "    values: {}\n",
                           this->memory_total_.value());
    }
    // maximum PCIe link speed
    if (this->pcie_link_max_speed_.has_value()) {
        str += std::format("  pcie_max_bandwidth:\n"
                           "    unit: \"MBPS\"\n"
                           "    values: {}\n",
                           this->pcie_link_max_speed_.value());
    }
    // memory bus width
    if (this->memory_bus_width_.has_value()) {
        str += std::format("  memory_bus_width:\n"
                           "    unit: \"Bit\"\n"
                           "    values: {}\n",
                           this->memory_bus_width_.value());
    }
    // maximum PCIe link generation
    if (this->max_pcie_link_generation_.has_value()) {
        str += std::format("  max_pcie_link_generation:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->max_pcie_link_generation_.value());
    }

    // free memory size
    if (this->memory_free_.has_value()) {
        str += std::format("  memory_free:\n"
                           "    unit: \"B\"\n"
                           "    values: [{}]\n",
                           detail::join(this->memory_free_.value(), ", "));
    }
    // used memory size
    if (this->memory_used_.has_value()) {
        str += std::format("  memory_used:\n"
                           "    unit: \"B\"\n"
                           "    values: [{}]\n",
                           detail::join(this->memory_used_.value(), ", "));
    }
    // PCIe link speed
    if (this->pcie_link_speed_.has_value()) {
        str += std::format("  pcie_bandwidth:\n"
                           "    unit: \"MBPS\"\n"
                           "    values: [{}]\n",
                           detail::join(this->pcie_link_speed_.value(), ", "));
    }
    // PCIe link width
    if (this->pcie_link_width_.has_value()) {
        str += std::format("  pcie_link_width:\n"
                           "    unit: \"int\"\n"
                           "    values: [{}]\n",
                           detail::join(this->pcie_link_width_.value(), ", "));
    }
    // PCIe link generation
    if (this->pcie_link_generation_.has_value()) {
        str += std::format("  pcie_link_generation:\n"
                           "    unit: \"int\"\n"
                           "    values: [{}]\n",
                           detail::join(this->pcie_link_generation_.value(), ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

std::ostream &operator<<(std::ostream &out, const nvml_memory_samples &samples) {
    return out << std::format("memory_total [B]: {}\n"
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
                              detail::join(detail::value_or_default(samples.get_memory_free()), ", "),
                              detail::join(detail::value_or_default(samples.get_memory_used()), ", "),
                              detail::join(detail::value_or_default(samples.get_pcie_link_speed()), ", "),
                              detail::join(detail::value_or_default(samples.get_pcie_link_width()), ", "),
                              detail::join(detail::value_or_default(samples.get_pcie_link_generation()), ", "));
}

//*************************************************************************************************************************************//
//                                                         temperature samples                                                         //
//*************************************************************************************************************************************//

std::string nvml_temperature_samples::generate_yaml_string() const {
    std::string str{ "temperature:\n" };

    // number of fans
    if (this->num_fans_.has_value()) {
        str += std::format("  num_fans:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->num_fans_.value());
    }
    // min fan speed
    if (this->min_fan_speed_.has_value()) {
        str += std::format("  min_fan_speed:\n"
                           "    unit: \"percentage\"\n"
                           "    values: {}\n",
                           this->min_fan_speed_.value());
    }
    // max fan speed
    if (this->max_fan_speed_.has_value()) {
        str += std::format("  max_fan_speed:\n"
                           "    unit: \"percentage\"\n"
                           "    values: {}\n",
                           this->max_fan_speed_.value());
    }
    // temperature threshold GPU max
    if (this->temperature_threshold_gpu_max_.has_value()) {
        str += std::format("  temperature_gpu_max:\n"
                           "    unit: \"°C\"\n"
                           "    values: {}\n",
                           this->temperature_threshold_gpu_max_.value());
    }
    // temperature threshold memory max
    if (this->temperature_threshold_mem_max_.has_value()) {
        str += std::format("  temperature_mem_max:\n"
                           "    unit: \"°C\"\n"
                           "    values: {}\n",
                           this->temperature_threshold_mem_max_.value());
    }

    // fan speed
    if (this->fan_speed_.has_value()) {
        str += std::format("  fan_speed:\n"
                           "    unit: \"percentage\"\n"
                           "    values: [{}]\n",
                           detail::join(this->fan_speed_.value(), ", "));
    }
    // temperature GPU
    if (this->temperature_gpu_.has_value()) {
        str += std::format("  temperature_gpu:\n"
                           "    unit: \"°C\"\n"
                           "    values: [{}]\n",
                           detail::join(this->temperature_gpu_.value(), ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

std::ostream &operator<<(std::ostream &out, const nvml_temperature_samples &samples) {
    return out << std::format("num_fans [int]: {}\n"
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
                              detail::join(detail::value_or_default(samples.get_fan_speed()), ", "),
                              detail::join(detail::value_or_default(samples.get_temperature_gpu()), ", "));
}

}  // namespace hws
