/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hardware_sampling/cpu/cpu_samples.hpp"

#include "hardware_sampling/utility.hpp"  // hws::detail::{value_or_default, join}

#include <array>        // std::array
#include <cstddef>      // std::size_t
#include <format>       // std::format
#include <ostream>      // std::ostream
#include <regex>        // std::regex, std::regex::extended, std::regex_match, std::regex_replace
#include <string>       // std::string
#include <string_view>  // std::string_view
#include <tuple>        // std::tuple

namespace hws {

//*************************************************************************************************************************************//
//                                                           general samples                                                           //
//*************************************************************************************************************************************//

std::string cpu_general_samples::generate_yaml_string() const {
    std::string str{ "general:\n" };

    // architecture
    if (this->architecture_.has_value()) {
        str += std::format("  architecture:\n"
                           "    unit: \"string\"\n"
                           "    values: \"{}\"\n",
                           this->architecture_.value());
    }
    // byte order
    if (this->byte_order_.has_value()) {
        str += std::format("  byte_order:\n"
                           "    unit: \"string\"\n"
                           "    values: \"{}\"\n",
                           this->byte_order_.value());
    }
    // number of threads including hyper-threads
    if (this->num_threads_.has_value()) {
        str += std::format("  num_threads:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->num_threads_.value());
    }
    // number of threads per core
    if (this->threads_per_core_.has_value()) {
        str += std::format("  threads_per_core:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->threads_per_core_.value());
    }
    // number of cores per socket
    if (this->cores_per_socket_.has_value()) {
        str += std::format("  cores_per_socket:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->cores_per_socket_.value());
    }
    // number of cores per socket
    if (this->num_sockets_.has_value()) {
        str += std::format("  num_sockets:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->num_sockets_.value());
    }
    // number of NUMA nodes
    if (this->numa_nodes_.has_value()) {
        str += std::format("  numa_nodes:\n"
                           "    unit: \"int\"\n"
                           "    values: {}\n",
                           this->numa_nodes_.value());
    }
    // the vendor specific ID
    if (this->vendor_id_.has_value()) {
        str += std::format("  vendor_id:\n"
                           "    unit: \"string\"\n"
                           "    values: \"{}\"\n",
                           this->vendor_id_.value());
    }
    // the CPU name
    if (this->name_.has_value()) {
        str += std::format("  name:\n"
                           "    unit: \"string\"\n"
                           "    values: \"{}\"\n",
                           this->name_.value());
    }
    // CPU specific flags (like SSE, AVX, ...)
    if (this->flags_.has_value()) {
        str += std::format("  flags:\n"
                           "    unit: \"string\"\n"
                           "    values: [{}]\n",
                           detail::join(this->flags_.value(), ", "));
    }

    // the percent the CPU was busy
    if (this->compute_utilization_.has_value()) {
        str += std::format("  compute_utilization:\n"
                           "    turbostat_name: \"Busy%\"\n"
                           "    unit: \"percentage\"\n"
                           "    values: [{}]\n",
                           detail::join(this->compute_utilization_.value(), ", "));
    }
    // the instructions per cycle count
    if (this->ipc_.has_value()) {
        str += std::format("  instructions_per_cycle:\n"
                           "    turbostat_name: \"IPC\"\n"
                           "    unit: \"float\"\n"
                           "    values: [{}]\n",
                           detail::join(this->ipc_.value(), ", "));
    }
    // the number of interrupts
    if (this->irq_.has_value()) {
        str += std::format("  interrupts:\n"
                           "    turbostat_name: \"IRQ\"\n"
                           "    unit: \"int\"\n"
                           "    values: [{}]\n",
                           detail::join(this->irq_.value(), ", "));
    }
    // the number of system management interrupts
    if (this->smi_.has_value()) {
        str += std::format("  system_management_interrupts:\n"
                           "    turbostat_name: \"SMI\"\n"
                           "    unit: \"int\"\n"
                           "    values: [{}]\n",
                           detail::join(this->smi_.value(), ", "));
    }
    // the number of times the CPU was in the poll state
    if (this->poll_.has_value()) {
        str += std::format("  polling_state:\n"
                           "    turbostat_name: \"POLL\"\n"
                           "    unit: \"int\"\n"
                           "    values: [{}]\n",
                           detail::join(this->poll_.value(), ", "));
    }
    // the percent the CPU was in the polling state
    if (this->poll_percent_.has_value()) {
        str += std::format("  polling_percentage:\n"
                           "    turbostat_name: \"POLL%\"\n"
                           "    unit: \"percentage\"\n"
                           "    values: [{}]\n",
                           detail::join(this->poll_percent_.value(), ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

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
                                  "compute_utilization [%]: [{}]\n"
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
                                  detail::join(detail::value_or_default(samples.get_compute_utilization()), ", "),
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

std::string cpu_clock_samples::generate_yaml_string() const {
    std::string str{ "clock:\n" };

    // true if frequency boost is enabled
    if (this->frequency_boost_.has_value()) {
        str += std::format("  frequency_boost:\n"
                           "    unit: \"bool\"\n"
                           "    values: {}\n",
                           this->frequency_boost_.value());
    }
    // the minimal CPU frequency
    if (this->min_frequency_.has_value()) {
        str += std::format("  min_cpu_frequency:\n"
                           "    unit: \"MHz\"\n"
                           "    values: {}\n",
                           this->min_frequency_.value());
    }
    // the maximum CPU frequency
    if (this->max_frequency_.has_value()) {
        str += std::format("  max_cpu_frequency:\n"
                           "    unit: \"MHz\"\n"
                           "    values: {}\n",
                           this->max_frequency_.value());
    }

    // the average CPU frequency
    if (this->average_frequency_.has_value()) {
        str += std::format("  average_frequency:\n"
                           "    turbostat_name: \"Avg_MHz\"\n"
                           "    unit: \"MHz\"\n"
                           "    values: [{}]\n",
                           detail::join(this->average_frequency_.value(), ", "));
    }
    // the average CPU frequency excluding idle time
    if (this->average_non_idle_frequency_.has_value()) {
        str += std::format("  average_non_idle_frequency:\n"
                           "    turbostat_name: \"Bzy_MHz\"\n"
                           "    unit: \"MHz\"\n"
                           "    values: [{}]\n",
                           detail::join(this->average_non_idle_frequency_.value(), ", "));
    }
    // the time stamp counter
    if (this->time_stamp_counter_.has_value()) {
        str += std::format("  time_stamp_counter:\n"
                           "    turbostat_name: \"TSC_MHz\"\n"
                           "    unit: \"MHz\"\n"
                           "    values: [{}]\n",
                           detail::join(this->time_stamp_counter_.value(), ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

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

std::string cpu_power_samples::generate_yaml_string() const {
    std::string str{ "power:\n" };

    // the package Watt
    if (this->package_watt_.has_value()) {
        str += std::format("  package_power:\n"
                           "    turbostat_name: \"PkgWatt\"\n"
                           "    unit: \"W\"\n"
                           "    values: [{}]\n",
                           detail::join(this->package_watt_.value(), ", "));
    }
    // the core Watt
    if (this->core_watt_.has_value()) {
        str += std::format("  core_power:\n"
                           "    turbostat_name: \"CorWatt\"\n"
                           "    unit: \"W\"\n"
                           "    values: [{}]\n",
                           detail::join(this->core_watt_.value(), ", "));
    }
    // the DRAM Watt
    if (this->ram_watt_.has_value()) {
        str += std::format("  dram_power:\n"
                           "    turbostat_name: \"RAMWatt\"\n"
                           "    unit: \"W\"\n"
                           "    values: [{}]\n",
                           detail::join(this->ram_watt_.value(), ", "));
    }
    // the percent of time when the RAPL package throttle was active
    if (this->package_rapl_throttle_percent_.has_value()) {
        str += std::format("  package_rapl_throttling:\n"
                           "    turbostat_name: \"PKG_%\"\n"
                           "    unit: \"percentage\"\n"
                           "    values: [{}]\n",
                           detail::join(this->package_rapl_throttle_percent_.value(), ", "));
    }
    // the percent of time when the RAPL DRAM throttle was active
    if (this->dram_rapl_throttle_percent_.has_value()) {
        str += std::format("  dram_rapl_throttling:\n"
                           "    turbostat_name: \"RAM_%\"\n"
                           "    unit: \"percentage\"\n"
                           "    values: [{}]\n",
                           detail::join(this->dram_rapl_throttle_percent_.value(), ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

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

std::string cpu_memory_samples::generate_yaml_string() const {
    std::string str{ "memory:\n" };

    // the size of the L1 data cache
    if (this->l1d_cache_.has_value()) {
        str += std::format("  cache_size_L1d:\n"
                           "    unit: \"string\"\n"
                           "    values: \"{}\"\n",
                           this->l1d_cache_.value());
    }
    // the size of the L1 instruction cache
    if (this->l1i_cache_.has_value()) {
        str += std::format("  cache_size_L1i:\n"
                           "    unit: \"string\"\n"
                           "    values: \"{}\"\n",
                           this->l1i_cache_.value());
    }
    // the size of the L2 cache
    if (this->l2_cache_.has_value()) {
        str += std::format("  cache_size_L2:\n"
                           "    unit: \"string\"\n"
                           "    values: \"{}\"\n",
                           this->l2_cache_.value());
    }
    // the size of the L3 cache
    if (this->l3_cache_.has_value()) {
        str += std::format("  cache_size_L3:\n"
                           "    unit: \"string\"\n"
                           "    values: \"{}\"\n",
                           this->l3_cache_.value());
    }

    // the total size of available memory
    if (this->memory_total_.has_value()) {
        str += std::format("  memory_total:\n"
                           "    unit: \"B\"\n"
                           "    values: {}\n",
                           this->memory_total_.value());
    }
    // the total size of the swap memory
    if (this->swap_memory_total_.has_value()) {
        str += std::format("  swap_memory_total:\n"
                           "    unit: \"B\"\n"
                           "    values: {}\n",
                           this->swap_memory_total_.value());
    }

    // the available free memory
    if (this->memory_free_.has_value()) {
        str += std::format("  memory_free:\n"
                           "    unit: \"B\"\n"
                           "    values: [{}]\n",
                           detail::join(this->memory_free_.value(), ", "));
    }
    // the used memory
    if (this->memory_used_.has_value()) {
        str += std::format("  memory_used:\n"
                           "    unit: \"B\"\n"
                           "    values: [{}]\n",
                           detail::join(this->memory_used_.value(), ", "));
    }
    // the available swap memory
    if (this->swap_memory_free_.has_value()) {
        str += std::format("  swap_memory_free:\n"
                           "    unit: \"B\"\n"
                           "    values: [{}]\n",
                           detail::join(this->swap_memory_free_.value(), ", "));
    }
    // the swap memory
    if (this->swap_memory_used_.has_value()) {
        str += std::format("  swap_memory_used:\n"
                           "    unit: \"B\"\n"
                           "    values: [{}]\n",
                           detail::join(this->swap_memory_used_.value(), ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

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

std::string cpu_temperature_samples::generate_yaml_string() const {
    std::string str{ "temperature:\n" };

    // the temperature of the cores
    if (this->core_temperature_.has_value()) {
        str += std::format("  per_core_temperature:\n"
                           "    turbostat_name: \"CoreTmp\"\n"
                           "    unit: \"°C\"\n"
                           "    values: [{}]\n",
                           detail::join(this->core_temperature_.value(), ", "));
    }
    // the percentage of time the core throttled due the temperature constraints
    if (this->core_throttle_percent_.has_value()) {
        str += std::format("  core_throttle_percentage:\n"
                           "    turbostat_name: \"CoreThr\"\n"
                           "    unit: \"percentage\"\n"
                           "    values: [{}]\n",
                           detail::join(this->core_throttle_percent_.value(), ", "));
    }
    // the temperature of the whole package
    if (this->package_temperature_.has_value()) {
        str += std::format("  per_package_temperature:\n"
                           "    turbostat_name: \"PkgTmp\"\n"
                           "    unit: \"°C\"\n"
                           "    values: [{}]\n",
                           detail::join(this->package_temperature_.value(), ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

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

std::string cpu_gfx_samples::generate_yaml_string() const {
    std::string str{ "integrated_gpu:\n" };

    // the percentage of time the iGPU was in the render state
    if (this->gfx_render_state_percent_.has_value()) {
        str += std::format("  graphics_render_state:\n"
                           "    turbostat_name: \"GFX%rc6\"\n"
                           "    unit: \"percentage\"\n"
                           "    values: [{}]\n",
                           detail::join(this->gfx_render_state_percent_.value(), ", "));
    }
    // the core frequency of the iGPU
    if (this->gfx_frequency_.has_value()) {
        str += std::format("  graphics_frequency:\n"
                           "    turbostat_name: \"GFXMHz\"\n"
                           "    unit: \"MHz\"\n"
                           "    values: [{}]\n",
                           detail::join(this->gfx_frequency_.value(), ", "));
    }
    // the average core frequency of the iGPU
    if (this->average_gfx_frequency_.has_value()) {
        str += std::format("  average_graphics_frequency:\n"
                           "    turbostat_name: \"GFXAMHz\"\n"
                           "    unit: \"MHz\"\n"
                           "    values: [{}]\n",
                           detail::join(this->average_gfx_frequency_.value(), ", "));
    }
    // the percentage of time the iGPU was in the c0 state
    if (this->gfx_state_c0_percent_.has_value()) {
        str += std::format("  gpu_state_c0:\n"
                           "    turbostat_name: \"GFX%C0\"\n"
                           "    unit: \"percentage\"\n"
                           "    values: [{}]\n",
                           detail::join(this->gfx_state_c0_percent_.value(), ", "));
    }
    // the percentage of time the CPU worked for the iGPU
    if (this->cpu_works_for_gpu_percent_.has_value()) {
        str += std::format("  cpu_works_for_gpu:\n"
                           "    turbostat_name: \"CPUGFX%\"\n"
                           "    unit: \"percentage\"\n"
                           "    values: [{}]\n",
                           detail::join(this->cpu_works_for_gpu_percent_.value(), ", "));
    }
    // the iGPU Watt
    if (this->gfx_watt_.has_value()) {
        str += std::format("  graphics_power:\n"
                           "    turbostat_name: \"GFXWatt\"\n"
                           "    unit: \"W\"\n"
                           "    values: [{}]\n",
                           detail::join(this->gfx_watt_.value(), ", "));
    }

    // remove last newline
    str.pop_back();

    return str;
}

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

std::string cpu_idle_states_samples::generate_yaml_string() const {
    std::string str{ "idle_states:\n" };

    // the percentage of time all CPUs were in the c0 state
    if (this->all_cpus_state_c0_percent_.has_value()) {
        str += std::format("  all_cpus_state_c0:\n"
                           "    turbostat_name: \"Totl%C0\"\n"
                           "    unit: \"percentage\"\n"
                           "    values: [{}]\n",
                           detail::join(this->all_cpus_state_c0_percent_.value(), ", "));
    }
    // the percentage of time any CPU was in the c0 state
    if (this->any_cpu_state_c0_percent_.has_value()) {
        str += std::format("  any_cpu_state_c0:\n"
                           "    turbostat_name: \"Any%C0\"\n"
                           "    unit: \"percentage\"\n"
                           "    values: [{}]\n",
                           detail::join(this->any_cpu_state_c0_percent_.value(), ", "));
    }
    // the percentage of time the CPUs were in the low power idle state
    if (this->low_power_idle_state_percent_.has_value()) {
        str += std::format("  lower_power_idle_state:\n"
                           "    turbostat_name: \"CPU%LPI\"\n"
                           "    unit: \"percentage\"\n"
                           "    values: [{}]\n",
                           detail::join(this->low_power_idle_state_percent_.value(), ", "));
    }
    // the percentage of time the CPUs were in the system low power idle state
    if (this->system_low_power_idle_state_percent_.has_value()) {
        str += std::format("  system_lower_power_idle_state:\n"
                           "    turbostat_name: \"SYS%LPI\"\n"
                           "    unit: \"percentage\"\n"
                           "    values: [{}]\n",
                           detail::join(this->system_low_power_idle_state_percent_.value(), ", "));
    }
    // the percentage of time the package was in the low power idle state
    if (this->package_low_power_idle_state_percent_.has_value()) {
        str += std::format("  package_lower_power_idle_state:\n"
                           "    turbostat_name: \"Pkg%LPI\"\n"
                           "    unit: \"percentage\"\n"
                           "    values: [{}]\n",
                           detail::join(this->package_low_power_idle_state_percent_.value(), ", "));
    }

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

                    str += std::format("  {}:\n"
                                       "    turbostat_name: \"{}\"\n"
                                       "    unit: \"{}\"\n"
                                       "    values: [{}]\n",
                                       entry_name_with_state,
                                       entry,
                                       entry_unit,
                                       detail::join(values, ", "));
                    break;
                }
            }
        }
    }

    // remove last newline
    str.pop_back();

    return str;
}

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
