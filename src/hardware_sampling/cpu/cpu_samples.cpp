/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hardware_sampling/cpu/cpu_samples.hpp"

#include "hardware_sampling/utility.hpp"  // hws::detail::{value_or_default, join}

#include <format>   // std::format
#include <ostream>  // std::ostream
#include <string>   // std::string

namespace hws {

//*************************************************************************************************************************************//
//                                                           general samples                                                           //
//*************************************************************************************************************************************//

std::ostream &operator<<(std::ostream &out, const cpu_general_samples &samples) {
    std::string str = std::format("architecture [string]: {}\n"
                                  "byte_order [string]: {}\n"
                                  "num_threads [int]: {}\n"
                                  "threads_per_core [int]: {}\n"
                                  "cores_per_socket [int]: {}\n"
                                  "num_sockets [int]: {}\n"
                                  "numa_nodes [int]: {}\n"
                                  "vendor_id [string]: {}\n"
                                  "name [string]: {}\n"
                                  "flags [string]: [{}]\n"
                                  "busy_percent [%]: [{}]\n"
                                  "ipc [float]: [{}]\n"
                                  "irq [int]: [{}]\n"
                                  "smi [int]: [{}]\n"
                                  "poll [int]: [{}]\n"
                                  "poll_percent [%]: [{}]",
                                  detail::value_or_default(samples.get_architecture()),
                                  detail::value_or_default(samples.get_byte_order()),
                                  detail::value_or_default(samples.get_num_threads()),
                                  detail::value_or_default(samples.get_threads_per_core()),
                                  detail::value_or_default(samples.get_cores_per_socket()),
                                  detail::value_or_default(samples.get_num_sockets()),
                                  detail::value_or_default(samples.get_numa_nodes()),
                                  detail::value_or_default(samples.get_vendor_id()),
                                  detail::value_or_default(samples.get_name()),
                                  detail::join(detail::value_or_default(samples.get_flags()), ", "),
                                  detail::join(detail::value_or_default(samples.get_busy_percent()), ", "),
                                  detail::join(detail::value_or_default(samples.get_ipc()), ", "),
                                  detail::join(detail::value_or_default(samples.get_irq()), ", "),
                                  detail::join(detail::value_or_default(samples.get_smi()), ", "),
                                  detail::join(detail::value_or_default(samples.get_poll()), ", "),
                                  detail::join(detail::value_or_default(samples.get_poll_percent()), ", "));

    // remove last newline
    str.pop_back();

    return out << str;
}

//*************************************************************************************************************************************//
//                                                            clock samples                                                            //
//*************************************************************************************************************************************//

std::ostream &operator<<(std::ostream &out, const cpu_clock_samples &samples) {
    return out << std::format("frequency_boost [bool]: {}\n"
                              "min_frequency [MHz]: {}\n"
                              "max_frequency [MHz]: {}\n"
                              "average_frequency [MHz]: [{}]\n"
                              "average_non_idle_frequency [MHz]: [{}]\n"
                              "time_stamp_counter [MHz]: [{}]",
                              detail::value_or_default(samples.get_frequency_boost()),
                              detail::value_or_default(samples.get_min_frequency()),
                              detail::value_or_default(samples.get_max_frequency()),
                              detail::join(detail::value_or_default(samples.get_average_frequency()), ", "),
                              detail::join(detail::value_or_default(samples.get_average_non_idle_frequency()), ", "),
                              detail::join(detail::value_or_default(samples.get_time_stamp_counter()), ", "));
}

//*************************************************************************************************************************************//
//                                                            power samples                                                            //
//*************************************************************************************************************************************//

std::ostream &operator<<(std::ostream &out, const cpu_power_samples &samples) {
    return out << std::format("package_watt [W]: [{}]\n"
                              "core_watt [W]: [{}]\n"
                              "ram_watt [W]: [{}]\n"
                              "package_rapl_throttle_percent [%]: [{}]\n"
                              "dram_rapl_throttle_percent [%]: [{}]",
                              detail::join(detail::value_or_default(samples.get_package_watt()), ", "),
                              detail::join(detail::value_or_default(samples.get_core_watt()), ", "),
                              detail::join(detail::value_or_default(samples.get_ram_watt()), ", "),
                              detail::join(detail::value_or_default(samples.get_package_rapl_throttle_percent()), ", "),
                              detail::join(detail::value_or_default(samples.get_dram_rapl_throttle_percent()), ", "));
}

//*************************************************************************************************************************************//
//                                                            memory samples                                                           //
//*************************************************************************************************************************************//

std::ostream &operator<<(std::ostream &out, const cpu_memory_samples &samples) {
    return out << std::format("l1d_cache [string]: {}\n"
                              "l1i_cache [string]: {}\n"
                              "l2_cache [string]: {}\n"
                              "l3_cache [string]: {}\n"
                              "memory_total [B]: {}\n"
                              "swap_memory_total [B]: {}\n"
                              "memory_free [B]: [{}]\n"
                              "memory_used [B]: [{}]\n"
                              "swap_memory_free [B]: [{}]\n"
                              "swap_memory_used [B]: [{}]",
                              detail::value_or_default(samples.get_l1d_cache()),
                              detail::value_or_default(samples.get_l1i_cache()),
                              detail::value_or_default(samples.get_l2_cache()),
                              detail::value_or_default(samples.get_l3_cache()),
                              detail::value_or_default(samples.get_memory_total()),
                              detail::value_or_default(samples.get_swap_memory_total()),
                              detail::join(detail::value_or_default(samples.get_memory_free()), ", "),
                              detail::join(detail::value_or_default(samples.get_memory_used()), ", "),
                              detail::join(detail::value_or_default(samples.get_swap_memory_free()), ", "),
                              detail::join(detail::value_or_default(samples.get_swap_memory_used()), ", "));
}

//*************************************************************************************************************************************//
//                                                         temperature samples                                                         //
//*************************************************************************************************************************************//

std::ostream &operator<<(std::ostream &out, const cpu_temperature_samples &samples) {
    return out << std::format("core_temperature [°C]: [{}]\n"
                              "core_throttle_percent [%]: [{}]\n"
                              "package_temperature [°C]: [{}]",
                              detail::join(detail::value_or_default(samples.get_core_temperature()), ", "),
                              detail::join(detail::value_or_default(samples.get_core_throttle_percent()), ", "),
                              detail::join(detail::value_or_default(samples.get_package_temperature()), ", "));
}

//*************************************************************************************************************************************//
//                                                          gfx (iGPU) samples                                                         //
//*************************************************************************************************************************************//

std::ostream &operator<<(std::ostream &out, const cpu_gfx_samples &samples) {
    return out << std::format("gfx_render_state_percent [%]: [{}]\n"
                              "gfx_frequency [MHz]: [{}]\n"
                              "average_gfx_frequency [MHz]: [{}]\n"
                              "gfx_state_c0_percent [%]: [{}]\n"
                              "cpu_works_for_gpu_percent [%]: [{}]\n"
                              "gfx_watt [W]: [{}]",
                              detail::join(detail::value_or_default(samples.get_gfx_render_state_percent()), ", "),
                              detail::join(detail::value_or_default(samples.get_gfx_frequency()), ", "),
                              detail::join(detail::value_or_default(samples.get_average_gfx_frequency()), ", "),
                              detail::join(detail::value_or_default(samples.get_gfx_state_c0_percent()), ", "),
                              detail::join(detail::value_or_default(samples.get_cpu_works_for_gpu_percent()), ", "),
                              detail::join(detail::value_or_default(samples.get_gfx_watt()), ", "));
}

//*************************************************************************************************************************************//
//                                                          idle state samples                                                         //
//*************************************************************************************************************************************//

std::ostream &operator<<(std::ostream &out, const cpu_idle_states_samples &samples) {
    std::string str = std::format("all_cpus_state_c0_percent [%]: [{}]\n"
                                  "any_cpu_state_c0_percent [%]: [{}]\n"
                                  "low_power_idle_state_percent [%]: [{}]\n"
                                  "system_low_power_idle_state_percent [%]: [{}]\n"
                                  "package_low_power_idle_state_percent [%]: [{}]\n",
                                  detail::join(detail::value_or_default(samples.get_all_cpus_state_c0_percent()), ", "),
                                  detail::join(detail::value_or_default(samples.get_any_cpu_state_c0_percent()), ", "),
                                  detail::join(detail::value_or_default(samples.get_low_power_idle_state_percent()), ", "),
                                  detail::join(detail::value_or_default(samples.get_system_low_power_idle_state_percent()), ", "),
                                  detail::join(detail::value_or_default(samples.get_package_low_power_idle_state_percent()), ", "));

    // add map entries
    if (samples.get_idle_states().has_value()) {
        for (const auto &[key, value] : samples.get_idle_states().value()) {
            str += std::format("{}: [{}]\n", key, detail::join(value, ", "));
        }
    }

    // remove last newline
    str.pop_back();

    return out << str;
}

}  // namespace hws
