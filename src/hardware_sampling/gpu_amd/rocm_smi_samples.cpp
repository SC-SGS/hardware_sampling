/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hardware_sampling/gpu_amd/rocm_smi_samples.hpp"

#include "hardware_sampling/utility.hpp"  // hws::detail::{value_or_default, join}

#include "rocm_smi/rocm_smi.h"  // RSMI_MAX_FAN_SPEED

#include <format>   // std::format
#include <ostream>  // std::ostream

namespace hws {

//*************************************************************************************************************************************//
//                                                           general samples                                                           //
//*************************************************************************************************************************************//

std::ostream &operator<<(std::ostream &out, const rocm_smi_general_samples &samples) {
    return out << std::format("name [string]: {}\n"
                              "performance_level [int]: [{}]\n"
                              "utilization_gpu [%]: [{}]\n"
                              "utilization_mem [%]: [{}]",
                              detail::value_or_default(samples.get_name()),
                              detail::join(detail::value_or_default(samples.get_performance_level()), ", "),
                              detail::join(detail::value_or_default(samples.get_utilization_gpu()), ", "),
                              detail::join(detail::value_or_default(samples.get_utilization_mem()), ", "));
}

//*************************************************************************************************************************************//
//                                                            clock samples                                                            //
//*************************************************************************************************************************************//

std::ostream &operator<<(std::ostream &out, const rocm_smi_clock_samples &samples) {
    return out << std::format("clock_system_min [Hz]: {}\n"
                              "clock_system_max [Hz]: {}\n"
                              "clock_socket_min [Hz]: {}\n"
                              "clock_socket_max [Hz]: {}\n"
                              "clock_memory_min [Hz]: {}\n"
                              "clock_memory_max [Hz]: {}\n"
                              "clock_system [Hz]: [{}]\n"
                              "clock_socket [Hz]: [{}]\n"
                              "clock_memory [Hz]: [{}]\n"
                              "overdrive_level [%]: [{}]\n"
                              "memory_overdrive_level [%]: [{}]",
                              detail::value_or_default(samples.get_clock_system_min()),
                              detail::value_or_default(samples.get_clock_system_max()),
                              detail::value_or_default(samples.get_clock_socket_min()),
                              detail::value_or_default(samples.get_clock_socket_max()),
                              detail::value_or_default(samples.get_clock_memory_min()),
                              detail::value_or_default(samples.get_clock_memory_max()),
                              detail::join(detail::value_or_default(samples.get_clock_system()), ", "),
                              detail::join(detail::value_or_default(samples.get_clock_socket()), ", "),
                              detail::join(detail::value_or_default(samples.get_clock_memory()), ", "),
                              detail::join(detail::value_or_default(samples.get_overdrive_level()), ", "),
                              detail::join(detail::value_or_default(samples.get_memory_overdrive_level()), ", "));
}

//*************************************************************************************************************************************//
//                                                            power samples                                                            //
//*************************************************************************************************************************************//

std::ostream &operator<<(std::ostream &out, const rocm_smi_power_samples &samples) {
    return out << std::format("power_default_cap [muW]: {}\n"
                              "power_cap [muW]: {}\n"
                              "power_type [string]: {}\n"
                              "available_power_profiles [string]: [{}]\n"
                              "power_usage [muW]: [{}]\n"
                              "power_total_energy_consumption [muJ]: [{}]\n"
                              "power_profile [string]: [{}]",
                              detail::value_or_default(samples.get_power_default_cap()),
                              detail::value_or_default(samples.get_power_cap()),
                              detail::value_or_default(samples.get_power_type()),
                              detail::join(detail::value_or_default(samples.get_available_power_profiles()), ", "),
                              detail::join(detail::value_or_default(samples.get_power_usage()), ", "),
                              detail::join(detail::value_or_default(samples.get_power_total_energy_consumption()), ", "),
                              detail::join(detail::value_or_default(samples.get_power_profile()), ", "));
}

//*************************************************************************************************************************************//
//                                                            memory samples                                                           //
//*************************************************************************************************************************************//

std::ostream &operator<<(std::ostream &out, const rocm_smi_memory_samples &samples) {
    return out << std::format("memory_total [B]: {}\n"
                              "visible_memory_total [B]: {}\n"
                              "min_num_pcie_lanes [int]: {}\n"
                              "max_num_pcie_lanes [int]: {}\n"
                              "memory_used [B]: [{}]\n"
                              "pcie_transfer_rate [T/s]: [{}]\n"
                              "num_pcie_lanes [int]: [{}]",
                              detail::value_or_default(samples.get_memory_total()),
                              detail::value_or_default(samples.get_visible_memory_total()),
                              detail::value_or_default(samples.get_min_num_pcie_lanes()),
                              detail::value_or_default(samples.get_max_num_pcie_lanes()),
                              detail::join(detail::value_or_default(samples.get_memory_used()), ", "),
                              detail::join(detail::value_or_default(samples.get_pcie_transfer_rate()), ", "),
                              detail::join(detail::value_or_default(samples.get_num_pcie_lanes()), ", "));
}

//*************************************************************************************************************************************//
//                                                         temperature samples                                                         //
//*************************************************************************************************************************************//

std::ostream &operator<<(std::ostream &out, const rocm_smi_temperature_samples &samples) {
    return out << std::format("num_fans [int]: {}\n"
                              "max_fan_speed [int]: {}\n"
                              "temperature_edge_min [m°C]: {}\n"
                              "temperature_edge_max [m°C]: {}\n"
                              "temperature_hotspot_min [m°C]: {}\n"
                              "temperature_hotspot_max [m°C]: {}\n"
                              "temperature_memory_min [m°C]: {}\n"
                              "temperature_memory_max [m°C]: {}\n"
                              "temperature_hbm_0_min [m°C]: {}\n"
                              "temperature_hbm_0_max [m°C]: {}\n"
                              "temperature_hbm_1_min [m°C]: {}\n"
                              "temperature_hbm_1_max [m°C]: {}\n"
                              "temperature_hbm_2_min [m°C]: {}\n"
                              "temperature_hbm_2_max [m°C]: {}\n"
                              "temperature_hbm_3_min [m°C]: {}\n"
                              "temperature_hbm_3_max [m°C]: {}\n"
                              "fan_speed [%]: [{}]\n"
                              "temperature_edge [m°C]: [{}]\n"
                              "temperature_hotspot [m°C]: [{}]\n"
                              "temperature_memory [m°C]: [{}]\n"
                              "temperature_hbm_0 [m°C]: [{}]\n"
                              "temperature_hbm_1 [m°C]: [{}]\n"
                              "temperature_hbm_2 [m°C]: [{}]\n"
                              "temperature_hbm_3 [m°C]: [{}]",
                              detail::value_or_default(samples.get_num_fans()),
                              detail::value_or_default(samples.get_max_fan_speed()),
                              detail::value_or_default(samples.get_temperature_edge_min()),
                              detail::value_or_default(samples.get_temperature_edge_max()),
                              detail::value_or_default(samples.get_temperature_hotspot_min()),
                              detail::value_or_default(samples.get_temperature_hotspot_max()),
                              detail::value_or_default(samples.get_temperature_memory_min()),
                              detail::value_or_default(samples.get_temperature_memory_max()),
                              detail::value_or_default(samples.get_temperature_hbm_0_min()),
                              detail::value_or_default(samples.get_temperature_hbm_0_max()),
                              detail::value_or_default(samples.get_temperature_hbm_1_min()),
                              detail::value_or_default(samples.get_temperature_hbm_1_max()),
                              detail::value_or_default(samples.get_temperature_hbm_2_min()),
                              detail::value_or_default(samples.get_temperature_hbm_2_max()),
                              detail::value_or_default(samples.get_temperature_hbm_3_min()),
                              detail::value_or_default(samples.get_temperature_hbm_3_max()),
                              detail::join(detail::value_or_default(samples.get_fan_speed()), ", "),
                              detail::join(detail::value_or_default(samples.get_temperature_edge()), ", "),
                              detail::join(detail::value_or_default(samples.get_temperature_hotspot()), ", "),
                              detail::join(detail::value_or_default(samples.get_temperature_memory()), ", "),
                              detail::join(detail::value_or_default(samples.get_temperature_hbm_0()), ", "),
                              detail::join(detail::value_or_default(samples.get_temperature_hbm_1()), ", "),
                              detail::join(detail::value_or_default(samples.get_temperature_hbm_2()), ", "),
                              detail::join(detail::value_or_default(samples.get_temperature_hbm_3()), ", "));
}

}  // namespace hws
