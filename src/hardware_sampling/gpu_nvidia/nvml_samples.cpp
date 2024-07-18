/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hardware_sampling/gpu_nvidia/nvml_samples.hpp"

#include "hardware_sampling/utility.hpp"  // hws::detail::{value_or_default, join}

#include <format>   // std::format
#include <ostream>  // std::ostream

namespace hws {

//*************************************************************************************************************************************//
//                                                           general samples                                                           //
//*************************************************************************************************************************************//

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
