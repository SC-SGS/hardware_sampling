/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hardware_sampling/cpu/cpu_samples.hpp"

#include "hardware_sampling/utility.hpp"  // hws::detail::{value_or_default, add_yaml_entry}

#include "fmt/format.h"  // fmt::format
#include "fmt/ranges.h"  // fmt::join
#include "ryml.hpp"      // ryml::NodeRef, ryml::MAP

#include <array>        // std::array
#include <cstddef>      // std::size_t
#include <ostream>      // std::ostream
#include <regex>        // std::regex, std::regex::extended, std::regex_match, std::regex_replace
#include <string>       // std::string
#include <string_view>  // std::string_view
#include <tuple>        // std::tuple

namespace hws {

//*************************************************************************************************************************************//
//                                                           general samples                                                           //
//*************************************************************************************************************************************//

void cpu_general_samples::add_yaml_entries(ryml::NodeRef &root) const {
    ryml::NodeRef general_samples = root["general"];
    general_samples |= ryml::MAP;

    // architecture
    detail::add_yaml_entry(general_samples, "architecture", "string", this->architecture_);
    // byte order
    detail::add_yaml_entry(general_samples, "byte_order", "string", this->byte_order_);
    // number of cores
    detail::add_yaml_entry(general_samples, "num_cores", "int", this->num_cores_);
    // number of threads including hyper-threads
    detail::add_yaml_entry(general_samples, "num_threads", "int", this->num_threads_);
    // number of threads per core
    detail::add_yaml_entry(general_samples, "threads_per_core", "int", this->threads_per_core_);
    // number of cores per socket
    detail::add_yaml_entry(general_samples, "cores_per_socket", "int", this->cores_per_socket_);
    // number of cores per socket
    detail::add_yaml_entry(general_samples, "num_sockets", "int", this->num_sockets_);
    // number of NUMA nodes
    detail::add_yaml_entry(general_samples, "numa_nodes", "int", this->numa_nodes_);
    // the vendor specific ID
    detail::add_yaml_entry(general_samples, "vendor_id", "string", this->vendor_id_);
    // the CPU name
    detail::add_yaml_entry(general_samples, "name", "string", this->name_);
    // CPU specific flags (like SSE, AVX, ...)
    detail::add_yaml_entry(general_samples, "flags", "string", this->flags_);

    // the percent the CPU was busy
    detail::add_yaml_entry(general_samples, "compute_utilization", "percentage", this->compute_utilization_, "Busy%");
    // the instructions per cycle count
    detail::add_yaml_entry(general_samples, "instructions_per_cycle", "float", this->ipc_, "IPC");
    // the number of interrupts
    detail::add_yaml_entry(general_samples, "interrupts", "int", this->irq_, "IRQ");
    // the number of system management interrupts
    detail::add_yaml_entry(general_samples, "system_management_interrupts", "int", this->smi_, "SMI");
    // the number of times the CPU was in the poll state
    detail::add_yaml_entry(general_samples, "polling_state", "int", this->poll_, "POLL");
    // the percent the CPU was in the polling state
    detail::add_yaml_entry(general_samples, "polling_percentage", "percentage", this->poll_percent_, "POLL%");
}

std::ostream &operator<<(std::ostream &out, const cpu_general_samples &samples) {
    std::string str = fmt::format("architecture [string]: {}\n"
                                  "byte_order [string]: {}\n"
                                  "num_cores [int]: {}\n"
                                  "num_threads [int]: {}\n"
                                  "threads_per_core [int]: {}\n"
                                  "cores_per_socket [int]: {}\n"
                                  "num_sockets [int]: {}\n"
                                  "numa_nodes [int]: {}\n"
                                  "vendor_id [string]: {}\n"
                                  "name [string]: {}\n"
                                  "flags [string]: [{}]\n"
                                  "compute_utilization [%]: [{}]\n"
                                  "ipc [float]: [{}]\n"
                                  "irq [int]: [{}]\n"
                                  "smi [int]: [{}]\n"
                                  "poll [int]: [{}]\n"
                                  "poll_percent [%]: [{}]",
                                  detail::value_or_default(samples.get_architecture()),
                                  detail::value_or_default(samples.get_byte_order()),
                                  detail::value_or_default(samples.get_num_cores()),
                                  detail::value_or_default(samples.get_num_threads()),
                                  detail::value_or_default(samples.get_threads_per_core()),
                                  detail::value_or_default(samples.get_cores_per_socket()),
                                  detail::value_or_default(samples.get_num_sockets()),
                                  detail::value_or_default(samples.get_numa_nodes()),
                                  detail::value_or_default(samples.get_vendor_id()),
                                  detail::value_or_default(samples.get_name()),
                                  fmt::join(detail::value_or_default(samples.get_flags()), ", "),
                                  fmt::join(detail::value_or_default(samples.get_compute_utilization()), ", "),
                                  fmt::join(detail::value_or_default(samples.get_ipc()), ", "),
                                  fmt::join(detail::value_or_default(samples.get_irq()), ", "),
                                  fmt::join(detail::value_or_default(samples.get_smi()), ", "),
                                  fmt::join(detail::value_or_default(samples.get_poll()), ", "),
                                  fmt::join(detail::value_or_default(samples.get_poll_percent()), ", "));

    // remove last newline
    str.pop_back();

    return out << str;
}

//*************************************************************************************************************************************//
//                                                            clock samples                                                            //
//*************************************************************************************************************************************//

void cpu_clock_samples::add_yaml_entries(ryml::NodeRef &root) const {
    ryml::NodeRef clock_samples = root["clock"];
    clock_samples |= ryml::MAP;

    // true if frequency boost is enabled
    detail::add_yaml_entry(clock_samples, "auto_boosted_clock_enabled", "bool", this->auto_boosted_clock_enabled_);
    // the minimal CPU frequency
    detail::add_yaml_entry(clock_samples, "clock_frequency_min", "MHz", this->clock_frequency_min_);
    // the maximum CPU frequency
    detail::add_yaml_entry(clock_samples, "clock_frequency_max", "MHz", this->clock_frequency_max_);

    // the average CPU frequency
    detail::add_yaml_entry(clock_samples, "clock_frequency", "MHz", this->clock_frequency_, "Avg_MHz");
    // the average CPU frequency excluding idle time
    detail::add_yaml_entry(clock_samples, "average_non_idle_clock_frequency", "MHz", this->average_non_idle_clock_frequency_, "Bzy_MHz");
    // the time stamp counter
    detail::add_yaml_entry(clock_samples, "time_stamp_counter", "MHz", this->time_stamp_counter_, "TSC_MHz");
}

std::ostream &operator<<(std::ostream &out, const cpu_clock_samples &samples) {
    return out << fmt::format("auto_boosted_clock_enabled [bool]: {}\n"
                              "clock_frequency_min [MHz]: {}\n"
                              "clock_frequency_max [MHz]: {}\n"
                              "clock_frequency [MHz]: [{}]\n"
                              "average_non_idle_clock_frequency [MHz]: [{}]\n"
                              "time_stamp_counter [MHz]: [{}]",
                              detail::value_or_default(samples.get_auto_boosted_clock_enabled()),
                              detail::value_or_default(samples.get_clock_frequency_min()),
                              detail::value_or_default(samples.get_clock_frequency_max()),
                              fmt::join(detail::value_or_default(samples.get_clock_frequency()), ", "),
                              fmt::join(detail::value_or_default(samples.get_average_non_idle_clock_frequency()), ", "),
                              fmt::join(detail::value_or_default(samples.get_time_stamp_counter()), ", "));
}

//*************************************************************************************************************************************//
//                                                            power samples                                                            //
//*************************************************************************************************************************************//

void cpu_power_samples::add_yaml_entries(ryml::NodeRef &root) const {
    ryml::NodeRef power_samples = root["power"];
    power_samples |= ryml::MAP;

    // power measurement type
    detail::add_yaml_entry(power_samples, "power_measurement_type", "string", this->power_measurement_type_);
    // the package Watt
    detail::add_yaml_entry(power_samples, "power_usage", "W", this->power_usage_, "PkgWatt");
    // total energy consumed
    detail::add_yaml_entry(power_samples, "power_total_energy_consumed", "J", this->power_total_energy_consumption_);

    // the core Watt
    detail::add_yaml_entry(power_samples, "core_power", "W", this->core_watt_, "CorWatt");
    // the DRAM Watt
    detail::add_yaml_entry(power_samples, "dram_power", "W", this->ram_watt_, "RAMWatt");
    // the percent of time when the RAPL package throttle was active
    detail::add_yaml_entry(power_samples, "package_rapl_throttling", "percentage", this->package_rapl_throttle_percent_, "PKG_%");
    // the percent of time when the RAPL DRAM throttle was active
    detail::add_yaml_entry(power_samples, "dram_rapl_throttling", "percentage", this->dram_rapl_throttle_percent_, "RAM_%");
}

std::ostream &operator<<(std::ostream &out, const cpu_power_samples &samples) {
    return out << fmt::format("power_measurement_type [string]: {}\n"
                              "power_usage [W]: [{}]\n"
                              "power_total_energy_consumption [J]: [{}]\n"
                              "core_watt [W]: [{}]\n"
                              "ram_watt [W]: [{}]\n"
                              "package_rapl_throttle_percent [%]: [{}]\n"
                              "dram_rapl_throttle_percent [%]: [{}]",
                              detail::value_or_default(samples.get_power_measurement_type()),
                              fmt::join(detail::value_or_default(samples.get_power_usage()), ", "),
                              fmt::join(detail::value_or_default(samples.get_power_total_energy_consumption()), ", "),
                              fmt::join(detail::value_or_default(samples.get_core_watt()), ", "),
                              fmt::join(detail::value_or_default(samples.get_ram_watt()), ", "),
                              fmt::join(detail::value_or_default(samples.get_package_rapl_throttle_percent()), ", "),
                              fmt::join(detail::value_or_default(samples.get_dram_rapl_throttle_percent()), ", "));
}

//*************************************************************************************************************************************//
//                                                            memory samples                                                           //
//*************************************************************************************************************************************//

void cpu_memory_samples::add_yaml_entries(ryml::NodeRef &root) const {
    ryml::NodeRef memory_samples = root["memory"];
    memory_samples |= ryml::MAP;

    // the size of the L1 data cache
    detail::add_yaml_entry(memory_samples, "cache_size_L1d", "string", this->cache_size_L1d_);
    // the size of the L1 instruction cache
    detail::add_yaml_entry(memory_samples, "cache_size_L1i", "string", this->cache_size_L1i_);
    // the size of the L2 cache
    detail::add_yaml_entry(memory_samples, "cache_size_L2", "string", this->cache_size_L2_);
    // the size of the L3 cache
    detail::add_yaml_entry(memory_samples, "cache_size_L3", "string", this->cache_size_L3_);

    // the total size of available memory
    detail::add_yaml_entry(memory_samples, "memory_total", "B", this->memory_total_);
    // the total size of the swap memory
    detail::add_yaml_entry(memory_samples, "swap_memory_total", "B", this->swap_memory_total_);

    // the used memory
    detail::add_yaml_entry(memory_samples, "memory_used", "B", this->memory_used_);
    // the available free memory
    detail::add_yaml_entry(memory_samples, "memory_free", "B", this->memory_free_);
    // the swap memory
    detail::add_yaml_entry(memory_samples, "swap_memory_used", "B", this->swap_memory_used_);
    // the available swap memory
    detail::add_yaml_entry(memory_samples, "swap_memory_free", "B", this->swap_memory_free_);
}

std::ostream &operator<<(std::ostream &out, const cpu_memory_samples &samples) {
    return out << fmt::format("cache_size_L1d [string]: {}\n"
                              "cache_size_L1i [string]: {}\n"
                              "cache_size_L2 [string]: {}\n"
                              "cache_size_L3 [string]: {}\n"
                              "memory_total [B]: {}\n"
                              "swap_memory_total [B]: {}\n"
                              "memory_used [B]: [{}]\n"
                              "memory_free [B]: [{}]\n"
                              "swap_memory_used [B]: [{}]\n"
                              "swap_memory_free [B]: [{}]",
                              detail::value_or_default(samples.get_cache_size_L1d()),
                              detail::value_or_default(samples.get_cache_size_L1i()),
                              detail::value_or_default(samples.get_cache_size_L2()),
                              detail::value_or_default(samples.get_cache_size_L3()),
                              detail::value_or_default(samples.get_memory_total()),
                              detail::value_or_default(samples.get_swap_memory_total()),
                              fmt::join(detail::value_or_default(samples.get_memory_used()), ", "),
                              fmt::join(detail::value_or_default(samples.get_memory_free()), ", "),
                              fmt::join(detail::value_or_default(samples.get_swap_memory_used()), ", "),
                              fmt::join(detail::value_or_default(samples.get_swap_memory_free()), ", "));
}

//*************************************************************************************************************************************//
//                                                         temperature samples                                                         //
//*************************************************************************************************************************************//

void cpu_temperature_samples::add_yaml_entries(ryml::NodeRef &root) const {
    ryml::NodeRef temperature_samples = root["temperature"];
    temperature_samples |= ryml::MAP;

    // the temperature of the whole package
    detail::add_yaml_entry(temperature_samples, "temperature", "°C", this->temperature_, "PkgTmp");
    // the temperature of the cores
    detail::add_yaml_entry(temperature_samples, "core_temperature", "°C", this->core_temperature_, "CoreTmp");
    // the percentage of time the core throttled due the temperature constraints
    detail::add_yaml_entry(temperature_samples, "core_throttle_percentage", "percentage", this->core_throttle_percent_, "CoreThr");
}

std::ostream &operator<<(std::ostream &out, const cpu_temperature_samples &samples) {
    return out << fmt::format("temperature [°C]: [{}]\n"
                              "core_temperature [°C]: [{}]\n"
                              "core_throttle_percent [%]: [{}]",
                              fmt::join(detail::value_or_default(samples.get_temperature()), ", "),
                              fmt::join(detail::value_or_default(samples.get_core_temperature()), ", "),
                              fmt::join(detail::value_or_default(samples.get_core_throttle_percent()), ", "));
}

//*************************************************************************************************************************************//
//                                                          gfx (iGPU) samples                                                         //
//*************************************************************************************************************************************//

void cpu_gfx_samples::add_yaml_entries(ryml::NodeRef &root) const {
    ryml::NodeRef gfx_samples = root["integrated_gpu"];
    gfx_samples |= ryml::MAP;

    // the percentage of time the iGPU was in the render state
    detail::add_yaml_entry(gfx_samples, "graphics_render_state", "percentage", this->gfx_render_state_percent_, "GFX%rc6");
    // the core frequency of the iGPU
    detail::add_yaml_entry(gfx_samples, "graphics_frequency", "MHz", this->gfx_frequency_, "GFXMHz");
    // the average core frequency of the iGPU
    detail::add_yaml_entry(gfx_samples, "average_graphics_frequency", "MHz", this->average_gfx_frequency_, "GFXAMHz");
    // the percentage of time the iGPU was in the c0 state
    detail::add_yaml_entry(gfx_samples, "gpu_state_c0", "percentage", this->gfx_state_c0_percent_, "GFX%C0");
    // the percentage of time the CPU worked for the iGPU
    detail::add_yaml_entry(gfx_samples, "cpu_works_for_gpu", "percentage", this->cpu_works_for_gpu_percent_, "CPUGFX%");
    // the iGPU Watt
    detail::add_yaml_entry(gfx_samples, "graphics_power", "W", this->gfx_watt_, "GFXWatt");
}

std::ostream &operator<<(std::ostream &out, const cpu_gfx_samples &samples) {
    return out << fmt::format("gfx_render_state_percent [%]: [{}]\n"
                              "gfx_frequency [MHz]: [{}]\n"
                              "average_gfx_frequency [MHz]: [{}]\n"
                              "gfx_state_c0_percent [%]: [{}]\n"
                              "cpu_works_for_gpu_percent [%]: [{}]\n"
                              "gfx_watt [W]: [{}]",
                              fmt::join(detail::value_or_default(samples.get_gfx_render_state_percent()), ", "),
                              fmt::join(detail::value_or_default(samples.get_gfx_frequency()), ", "),
                              fmt::join(detail::value_or_default(samples.get_average_gfx_frequency()), ", "),
                              fmt::join(detail::value_or_default(samples.get_gfx_state_c0_percent()), ", "),
                              fmt::join(detail::value_or_default(samples.get_cpu_works_for_gpu_percent()), ", "),
                              fmt::join(detail::value_or_default(samples.get_gfx_watt()), ", "));
}

//*************************************************************************************************************************************//
//                                                          idle state samples                                                         //
//*************************************************************************************************************************************//

void cpu_idle_states_samples::add_yaml_entries(ryml::NodeRef &root) const {
    ryml::NodeRef idle_states_samples = root["idle_states"];
    idle_states_samples |= ryml::MAP;

    // the percentage of time all CPUs were in the c0 state
    detail::add_yaml_entry(idle_states_samples, "all_cpus_state_c0", "percentage", this->all_cpus_state_c0_percent_, "Totl%C0");
    // the percentage of time any CPU was in the c0 state
    detail::add_yaml_entry(idle_states_samples, "any_cpu_state_c0", "percentage", this->any_cpu_state_c0_percent_, "Any%C0");
    // the percentage of time the CPUs were in the low power idle state
    detail::add_yaml_entry(idle_states_samples, "lower_power_idle_state", "percentage", this->low_power_idle_state_percent_, "CPU%LPI");
    // the percentage of time the CPUs were in the system low power idle state
    detail::add_yaml_entry(idle_states_samples, "system_lower_power_idle_state", "percentage", this->system_low_power_idle_state_percent_, "SYS%LPI");
    // the percentage of time the package was in the low power idle state
    detail::add_yaml_entry(idle_states_samples, "package_lower_power_idle_state", "percentage", this->package_low_power_idle_state_percent_, "Pkg%LPI");

    // the other core idle states
    if (this->idle_states_.has_value()) {
        constexpr static std::array<std::tuple<std::string_view, std::string_view, std::string_view, std::size_t, std::size_t>, 5> turbostat_regex{
            std::tuple{ "CPU%[0-9a-zA-Z]+", "cpu_idle_state_PLACEHOLDER_percentage", "percentage", 4, 0 },
            std::tuple{ "Pkg%[0-9a-zA-Z]+", "package_idle_state_PLACEHOLDER_percentage", "percentage", 4, 0 },
            std::tuple{ "Pk%[0-9a-zA-Z]+", "package_idle_state_PLACEHOLDER_percentage", "percentage", 3, 0 },
            std::tuple{ "C[0-9a-zA-Z]+%", "idle_state_PLACEHOLDER_percentage", "percentage", 1, 1 },
            std::tuple{ "C[0-9a-zA-Z]+", "idle_state_PLACEHOLDER", "int", 1, 0 }
        };

        for (const auto &[entry, values] : this->idle_states_.value()) {
            // test all regular expressions one after another
            for (const auto &[regex, entry_name_placeholder, entry_unit, prefix_size, suffix_size] : turbostat_regex) {
                const std::regex reg{ std::string{ regex }, std::regex::extended };
                const std::regex placeholder_reg{ std::string{ "PLACEHOLDER" } };

                if (std::regex_match(entry, reg)) {
                    // remove specified prefix and suffix from state
                    std::string_view state{ entry };
                    state.remove_prefix(prefix_size);
                    state.remove_suffix(suffix_size);

                    // assemble better category name
                    std::string entry_name_with_state{};
                    std::regex_replace(std::back_inserter(entry_name_with_state), entry_name_placeholder.begin(), entry_name_placeholder.end(), placeholder_reg, std::string{ state });

                    const std::string entry_unit_str{ entry_unit };
                    const std::string entry_str{ entry };

                    detail::add_yaml_entry(idle_states_samples, entry_name_with_state.c_str(), entry_unit_str.c_str(), std::make_optional(values), entry_str.c_str());
                }
            }
        }
    }
}

std::ostream &operator<<(std::ostream &out, const cpu_idle_states_samples &samples) {
    std::string str = fmt::format("all_cpus_state_c0_percent [%]: [{}]\n"
                                  "any_cpu_state_c0_percent [%]: [{}]\n"
                                  "low_power_idle_state_percent [%]: [{}]\n"
                                  "system_low_power_idle_state_percent [%]: [{}]\n"
                                  "package_low_power_idle_state_percent [%]: [{}]\n",
                                  fmt::join(detail::value_or_default(samples.get_all_cpus_state_c0_percent()), ", "),
                                  fmt::join(detail::value_or_default(samples.get_any_cpu_state_c0_percent()), ", "),
                                  fmt::join(detail::value_or_default(samples.get_low_power_idle_state_percent()), ", "),
                                  fmt::join(detail::value_or_default(samples.get_system_low_power_idle_state_percent()), ", "),
                                  fmt::join(detail::value_or_default(samples.get_package_low_power_idle_state_percent()), ", "));

    // add map entries
    if (samples.get_idle_states().has_value()) {
        for (const auto &[key, value] : samples.get_idle_states().value()) {
            str += fmt::format("{}: [{}]\n", key, fmt::join(value, ", "));
        }
    }

    // remove last newline
    str.pop_back();

    return out << str;
}

}  // namespace hws
