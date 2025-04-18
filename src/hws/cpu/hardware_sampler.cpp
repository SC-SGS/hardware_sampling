/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hws/cpu/hardware_sampler.hpp"

#include "hws/cpu/cpu_samples.hpp"   // hws::{cpu_general_samples, clock_samples, power_samples, memory_samples, temperature_samples, gfx_samples, idle_state_samples}
#include "hws/cpu/utility.hpp"       // HWS_SUBPROCESS_ERROR_CHECK, hws::detail::run_subprocess
#include "hws/hardware_sampler.hpp"  // hws::tracking::hardware_sampler
#include "hws/sample_category.hpp"   // hws::sample_category
#include "hws/utility.hpp"           // hws::detail::{split, split_as, trim, convert_to, starts_with}

#include "fmt/chrono.h"  // direct formatting of std::chrono types
#include "fmt/format.h"  // fmt::format
#include "fmt/ranges.h"  // fmt::join

#include <cassert>        // assert
#include <chrono>         // std::chrono::{steady_clock, milliseconds}
#include <cstddef>        // std::size_t
#include <exception>      // std::exception, std::terminate
#include <ios>            // std::ios_base
#include <iostream>       // std::cerr, std::endl
#include <optional>       // std::make_optional
#include <ostream>        // std::ostream
#include <regex>          // std::regex, std::regex::extended, std::regex_match, std::regex_replace
#include <stdexcept>      // std::runtime_error
#include <string>         // std::string
#include <string_view>    // std::string_view
#include <thread>         // std::this_thread
#include <unordered_map>  // std::unordered_map
#include <vector>         // std::vector

namespace hws {

cpu_hardware_sampler::cpu_hardware_sampler(const sample_category category) :
    cpu_hardware_sampler{ HWS_SAMPLING_INTERVAL, category } { }

cpu_hardware_sampler::cpu_hardware_sampler(const std::chrono::milliseconds sampling_interval, const sample_category category) :
    hardware_sampler{ sampling_interval, category } { }

cpu_hardware_sampler::~cpu_hardware_sampler() {
    try {
        // if this hardware sampler is still sampling, stop it
        if (this->has_sampling_started() && !this->has_sampling_stopped()) {
            this->stop_sampling();
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::terminate();
    }
}

void cpu_hardware_sampler::sampling_loop() {
    //
    // add samples where we only have to retrieve the value once
    //

    this->add_time_point(std::chrono::steady_clock::now());

#if defined(HWS_VIA_LSCPU_ENABLED)
    {
        const std::string lscpu_output = detail::run_subprocess("lscpu");
        const std::vector<std::string_view> lscpu_lines = detail::split(detail::trim(lscpu_output), '\n');

        for (std::string_view line : lscpu_lines) {
            line = detail::trim(line);
            // extract the value
            std::string_view value{ line };
            value.remove_prefix(value.find_first_of(":") + 1);
            value = detail::trim(value);

            // check the lines if the start with an entry that we want to sample
            if (this->sample_category_enabled(sample_category::general)) {
                if (detail::starts_with(line, "Architecture")) {
                    general_samples_.architecture_ = detail::convert_to<decltype(general_samples_.architecture_)::value_type>(value);
                } else if (detail::starts_with(line, "Byte Order")) {
                    general_samples_.byte_order_ = detail::convert_to<decltype(general_samples_.byte_order_)::value_type>(value);
                } else if (detail::starts_with(line, "CPU(s)") && detail::is_integer(value)) {
                    general_samples_.num_threads_ = detail::convert_to<decltype(general_samples_.num_threads_)::value_type>(value);
                } else if (detail::starts_with(line, "Thread(s) per core") && detail::is_integer(value)) {
                    general_samples_.threads_per_core_ = detail::convert_to<decltype(general_samples_.threads_per_core_)::value_type>(value);
                } else if (detail::starts_with(line, "Core(s) per socket") && detail::is_integer(value)) {
                    general_samples_.cores_per_socket_ = detail::convert_to<decltype(general_samples_.cores_per_socket_)::value_type>(value);
                } else if (detail::starts_with(line, "Socket(s)") && detail::is_integer(value)) {
                    general_samples_.num_sockets_ = detail::convert_to<decltype(general_samples_.num_sockets_)::value_type>(value);
                } else if (detail::starts_with(line, "NUMA node(s)") && detail::is_integer(value)) {
                    general_samples_.numa_nodes_ = detail::convert_to<decltype(general_samples_.numa_nodes_)::value_type>(value);
                } else if (detail::starts_with(line, "Vendor ID")) {
                    general_samples_.vendor_id_ = detail::convert_to<decltype(general_samples_.vendor_id_)::value_type>(value);
                } else if (detail::starts_with(line, "Model name")) {
                    general_samples_.name_ = detail::convert_to<decltype(general_samples_.name_)::value_type>(value);
                } else if (detail::starts_with(line, "Flags")) {
                    general_samples_.flags_ = detail::split_as<decltype(general_samples_.flags_)::value_type::value_type>(value, ' ');
                }
            }
            if (this->sample_category_enabled(sample_category::clock)) {
                if (detail::starts_with(line, "Frequency boost")) {
                    clock_samples_.auto_boosted_clock_enabled_ = value == "enabled";
                } else if (detail::starts_with(line, "CPU max MHz")) {
                    clock_samples_.clock_frequency_max_ = detail::convert_to<decltype(clock_samples_.clock_frequency_max_)::value_type>(value);
                } else if (detail::starts_with(line, "CPU min MHz")) {
                    clock_samples_.clock_frequency_min_ = detail::convert_to<decltype(clock_samples_.clock_frequency_min_)::value_type>(value);
                }
            }
            if (this->sample_category_enabled(sample_category::memory)) {
                if (detail::starts_with(line, "L1d cache")) {
                    memory_samples_.cache_size_L1d_ = detail::convert_to<decltype(memory_samples_.cache_size_L1d_)::value_type>(value);
                } else if (detail::starts_with(line, "L1i cache")) {
                    memory_samples_.cache_size_L1i_ = detail::convert_to<decltype(memory_samples_.cache_size_L1i_)::value_type>(value);
                } else if (detail::starts_with(line, "L2 cache")) {
                    memory_samples_.cache_size_L2_ = detail::convert_to<decltype(memory_samples_.cache_size_L2_)::value_type>(value);
                } else if (detail::starts_with(line, "L3 cache")) {
                    memory_samples_.cache_size_L3_ = detail::convert_to<decltype(memory_samples_.cache_size_L3_)::value_type>(value);
                }
            }
        }

        if (this->sample_category_enabled(sample_category::general)) {
            // check if the number of cores can be derived from the otherwise found values
            if (general_samples_.num_threads_.has_value() && general_samples_.threads_per_core_.has_value()) {
                general_samples_.num_cores_ = general_samples_.num_threads_.value() / general_samples_.threads_per_core_.value();
            }
        }
    }
#endif

#if defined(HWS_VIA_FREE_ENABLED)
    const std::regex whitespace_replace_reg{ "[ ]+", std::regex::extended };
    if (this->sample_category_enabled(sample_category::memory)) {
        std::string free_output = detail::run_subprocess("free -b");
        free_output = std::regex_replace(free_output, whitespace_replace_reg, " ");
        const std::vector<std::string_view> free_lines = detail::split(detail::trim(free_output), '\n');
        assert((free_lines.size() >= 3) && "Must read more than three lines, but fewer were read!");

        // read memory information
        const std::vector<std::string_view> memory_data = detail::split(free_lines[1], ' ');
        memory_samples_.memory_total_ = detail::convert_to<decltype(memory_samples_.memory_total_)::value_type>(memory_data[1]);
        memory_samples_.memory_used_ = decltype(memory_samples_.memory_used_)::value_type{ detail::convert_to<decltype(memory_samples_.memory_used_)::value_type::value_type>(memory_data[2]) };
        memory_samples_.memory_free_ = decltype(memory_samples_.memory_free_)::value_type{ detail::convert_to<decltype(memory_samples_.memory_free_)::value_type::value_type>(memory_data[3]) };

        // read swap information
        const std::vector<std::string_view> swap_data = detail::split(free_lines[2], ' ');
        memory_samples_.swap_memory_total_ = detail::convert_to<decltype(memory_samples_.swap_memory_total_)::value_type>(swap_data[1]);
        memory_samples_.swap_memory_used_ = decltype(memory_samples_.swap_memory_used_)::value_type{ detail::convert_to<decltype(memory_samples_.swap_memory_used_)::value_type::value_type>(swap_data[2]) };
        memory_samples_.swap_memory_free_ = decltype(memory_samples_.swap_memory_free_)::value_type{ detail::convert_to<decltype(memory_samples_.swap_memory_free_)::value_type::value_type>(swap_data[3]) };
    }
#endif

#if defined(HWS_VIA_TURBOSTAT_ENABLED)

    // -n, --num_iterations     number of the measurement iterations
    // -i, --interval           sampling interval in seconds (decimal number)
    // -S, --Summary            limits output to 1-line per interval
    // -q, --quiet              skip decoding system configuration header

    // get header information
    #if defined(HWS_VIA_TURBOSTAT_ROOT)
    // run with sudo
    const std::string_view turbostat_command_line = "sudo turbostat -n 1 -i 0.001 -S -q";
    #else
    // run without sudo
    const std::string_view turbostat_command_line = "turbostat -n 1 -i 0.001 -S -q";
    #endif

    {
        // run turbostat
        const std::string turbostat_output = detail::run_subprocess(turbostat_command_line);

        // retrieve the turbostat data
        const std::vector<std::string_view> data = detail::split(detail::trim(turbostat_output), '\n');
        assert((data.size() >= 2) && "Must read at least two lines!");
        const std::vector<std::string_view> header = detail::split(data[0], '\t');
        const std::vector<std::string_view> values = detail::split(data[1], '\t');

        for (std::size_t i = 0; i < header.size(); ++i) {
            // general samples
            if (header[i] == "Busy%") {
                if (this->sample_category_enabled(sample_category::general)) {
                    using vector_type = decltype(general_samples_.compute_utilization_)::value_type;
                    general_samples_.compute_utilization_ = vector_type{ detail::convert_to<typename vector_type::value_type>(values[i]) };
                }
                continue;
            } else if (header[i] == "IPC") {
                if (this->sample_category_enabled(sample_category::general)) {
                    using vector_type = decltype(general_samples_.ipc_)::value_type;
                    general_samples_.ipc_ = vector_type{ detail::convert_to<typename vector_type::value_type>(values[i]) };
                }
                continue;
            } else if (header[i] == "IRQ") {
                if (this->sample_category_enabled(sample_category::general)) {
                    using vector_type = decltype(general_samples_.irq_)::value_type;
                    general_samples_.irq_ = vector_type{ detail::convert_to<typename vector_type::value_type>(values[i]) };
                }
                continue;
            } else if (header[i] == "SMI") {
                if (this->sample_category_enabled(sample_category::general)) {
                    using vector_type = decltype(general_samples_.smi_)::value_type;
                    general_samples_.smi_ = vector_type{ detail::convert_to<typename vector_type::value_type>(values[i]) };
                }
            } else if (header[i] == "POLL") {
                if (this->sample_category_enabled(sample_category::general)) {
                    using vector_type = decltype(general_samples_.poll_)::value_type;
                    general_samples_.poll_ = vector_type{ detail::convert_to<typename vector_type::value_type>(values[i]) };
                }
                continue;
            } else if (header[i] == "POLL%") {
                if (this->sample_category_enabled(sample_category::general)) {
                    using vector_type = decltype(general_samples_.poll_percent_)::value_type;
                    general_samples_.poll_percent_ = vector_type{ detail::convert_to<typename vector_type::value_type>(values[i]) };
                }
                continue;
            }

            // clock related samples
            if (header[i] == "Avg_MHz") {
                if (this->sample_category_enabled(sample_category::clock)) {
                    using vector_type = decltype(clock_samples_.clock_frequency_)::value_type;
                    clock_samples_.clock_frequency_ = vector_type{ detail::convert_to<typename vector_type::value_type>(values[i]) };
                }
                continue;
            } else if (header[i] == "Bzy_MHz") {
                if (this->sample_category_enabled(sample_category::clock)) {
                    using vector_type = decltype(clock_samples_.average_non_idle_clock_frequency_)::value_type;
                    clock_samples_.average_non_idle_clock_frequency_ = vector_type{ detail::convert_to<typename vector_type::value_type>(values[i]) };
                }
                continue;
            } else if (header[i] == "TSC_MHz") {
                if (this->sample_category_enabled(sample_category::clock)) {
                    using vector_type = decltype(clock_samples_.time_stamp_counter_)::value_type;
                    clock_samples_.time_stamp_counter_ = vector_type{ detail::convert_to<typename vector_type::value_type>(values[i]) };
                }
                continue;
            }

            // power related samples
            if (header[i] == "PkgWatt") {
                if (this->sample_category_enabled(sample_category::power)) {
                    using vector_type = decltype(power_samples_.power_usage_)::value_type;
                    power_samples_.power_usage_ = vector_type{ detail::convert_to<typename vector_type::value_type>(values[i]) };
                    power_samples_.power_measurement_type_ = "current/instant";
                    power_samples_.power_total_energy_consumption_ = decltype(power_samples_.power_total_energy_consumption_)::value_type{ 0 };
                }
                continue;
            } else if (header[i] == "CorWatt") {
                if (this->sample_category_enabled(sample_category::power)) {
                    using vector_type = decltype(power_samples_.core_watt_)::value_type;
                    power_samples_.core_watt_ = vector_type{ detail::convert_to<typename vector_type::value_type>(values[i]) };
                }
                continue;
            } else if (header[i] == "RAMWatt") {
                if (this->sample_category_enabled(sample_category::power)) {
                    using vector_type = decltype(power_samples_.ram_watt_)::value_type;
                    power_samples_.ram_watt_ = vector_type{ detail::convert_to<typename vector_type::value_type>(values[i]) };
                }
                continue;
            } else if (header[i] == "PKG_%") {
                if (this->sample_category_enabled(sample_category::power)) {
                    using vector_type = decltype(power_samples_.package_rapl_throttle_percent_)::value_type;
                    power_samples_.package_rapl_throttle_percent_ = vector_type{ detail::convert_to<typename vector_type::value_type>(values[i]) };
                }
                continue;
            } else if (header[i] == "RAM_%") {
                if (this->sample_category_enabled(sample_category::power)) {
                    using vector_type = decltype(power_samples_.dram_rapl_throttle_percent_)::value_type;
                    power_samples_.dram_rapl_throttle_percent_ = vector_type{ detail::convert_to<typename vector_type::value_type>(values[i]) };
                }
                continue;
            }

            // temperature related samples
            if (header[i] == "CoreTmp") {
                if (this->sample_category_enabled(sample_category::temperature)) {
                    using vector_type = decltype(temperature_samples_.core_temperature_)::value_type;
                    temperature_samples_.core_temperature_ = vector_type{ detail::convert_to<typename vector_type::value_type>(values[i]) };
                }
                continue;
            } else if (header[i] == "CoreThr") {
                if (this->sample_category_enabled(sample_category::temperature)) {
                    using vector_type = decltype(temperature_samples_.core_throttle_percent_)::value_type;
                    temperature_samples_.core_throttle_percent_ = vector_type{ detail::convert_to<typename vector_type::value_type>(values[i]) };
                }
                continue;
            } else if (header[i] == "PkgTmp") {
                if (this->sample_category_enabled(sample_category::temperature)) {
                    using vector_type = decltype(temperature_samples_.temperature_)::value_type;
                    temperature_samples_.temperature_ = vector_type{ detail::convert_to<typename vector_type::value_type>(values[i]) };
                }
                continue;
            }

            // gfx (iGPU) related samples
            if (header[i] == "GFX%rc6") {
                if (this->sample_category_enabled(sample_category::gfx)) {
                    using vector_type = decltype(gfx_samples_.gfx_render_state_percent_)::value_type;
                    gfx_samples_.gfx_render_state_percent_ = vector_type{ detail::convert_to<typename vector_type::value_type>(values[i]) };
                }
                continue;
            } else if (header[i] == "GFXMHz") {
                if (this->sample_category_enabled(sample_category::gfx)) {
                    using vector_type = decltype(gfx_samples_.gfx_frequency_)::value_type;
                    gfx_samples_.gfx_frequency_ = vector_type{ detail::convert_to<typename vector_type::value_type>(values[i]) };
                }
                continue;
            } else if (header[i] == "GFXAMHz") {
                if (this->sample_category_enabled(sample_category::gfx)) {
                    using vector_type = decltype(gfx_samples_.average_gfx_frequency_)::value_type;
                    gfx_samples_.average_gfx_frequency_ = vector_type{ detail::convert_to<typename vector_type::value_type>(values[i]) };
                }
                continue;
            } else if (header[i] == "GFX%C0") {
                if (this->sample_category_enabled(sample_category::gfx)) {
                    using vector_type = decltype(gfx_samples_.gfx_state_c0_percent_)::value_type;
                    gfx_samples_.gfx_state_c0_percent_ = vector_type{ detail::convert_to<typename vector_type::value_type>(values[i]) };
                }
                continue;
            } else if (header[i] == "CPUGFX%") {
                if (this->sample_category_enabled(sample_category::gfx)) {
                    using vector_type = decltype(gfx_samples_.cpu_works_for_gpu_percent_)::value_type;
                    gfx_samples_.cpu_works_for_gpu_percent_ = vector_type{ detail::convert_to<typename vector_type::value_type>(values[i]) };
                }
                continue;
            } else if (header[i] == "GFXWatt") {
                if (this->sample_category_enabled(sample_category::gfx)) {
                    using vector_type = decltype(gfx_samples_.gfx_watt_)::value_type;
                    gfx_samples_.gfx_watt_ = vector_type{ detail::convert_to<typename vector_type::value_type>(values[i]) };
                }
                continue;
            }

            // idle state related samples
            if (header[i] == "Totl%C0") {
                if (this->sample_category_enabled(sample_category::idle_state)) {
                    using vector_type = decltype(idle_state_samples_.all_cpus_state_c0_percent_)::value_type;
                    idle_state_samples_.all_cpus_state_c0_percent_ = vector_type{ detail::convert_to<typename vector_type::value_type>(values[i]) };
                }
                continue;
            } else if (header[i] == "Any%C0") {
                if (this->sample_category_enabled(sample_category::idle_state)) {
                    using vector_type = decltype(idle_state_samples_.any_cpu_state_c0_percent_)::value_type;
                    idle_state_samples_.any_cpu_state_c0_percent_ = vector_type{ detail::convert_to<typename vector_type::value_type>(values[i]) };
                }
                continue;
            } else if (header[i] == "CPU%LPI") {
                if (this->sample_category_enabled(sample_category::idle_state)) {
                    using vector_type = decltype(idle_state_samples_.low_power_idle_state_percent_)::value_type;
                    idle_state_samples_.low_power_idle_state_percent_ = vector_type{ detail::convert_to<typename vector_type::value_type>(values[i]) };
                }
                continue;
            } else if (header[i] == "SYS%LPI") {
                if (this->sample_category_enabled(sample_category::idle_state)) {
                    using vector_type = decltype(idle_state_samples_.system_low_power_idle_state_percent_)::value_type;
                    idle_state_samples_.system_low_power_idle_state_percent_ = vector_type{ detail::convert_to<typename vector_type::value_type>(values[i]) };
                }
                continue;
            } else if (header[i] == "Pkg%LPI") {
                if (this->sample_category_enabled(sample_category::idle_state)) {
                    using vector_type = decltype(idle_state_samples_.package_low_power_idle_state_percent_)::value_type;
                    idle_state_samples_.package_low_power_idle_state_percent_ = vector_type{ detail::convert_to<typename vector_type::value_type>(values[i]) };
                }
                continue;
            } else {
                if (this->sample_category_enabled(sample_category::idle_state)) {
                    // test against regex
                    const std::string header_str{ header[i] };
                    const std::regex reg{ std::string{ "CPU%[0-9a-zA-Z]+|Pkg%[0-9a-zA-Z]+|Pk%[0-9a-zA-Z]+|C[0-9a-zA-Z]+%|C[0-9a-zA-Z]+" }, std::regex::extended };
                    if (std::regex_match(header_str, reg)) {
                        // first time this branch is reached -> create optional value
                        if (!idle_state_samples_.idle_states_.has_value()) {
                            idle_state_samples_.idle_states_ = std::make_optional<typename cpu_idle_states_samples::map_type>();
                        }

                        using vector_type = cpu_idle_states_samples::map_type::mapped_type;
                        idle_state_samples_.idle_states_.value()[header_str] = vector_type{ detail::convert_to<typename vector_type::value_type>(values[i]) };
                    }
                }
                continue;
            }
        }
    }
#endif

    //
    // loop until stop_sampling() is called
    //

    while (!this->has_sampling_stopped()) {
        // only sample values if the sampler currently isn't paused
        if (this->is_sampling()) {
            // add current time point
            this->add_time_point(std::chrono::steady_clock::now());

#if defined(HWS_VIA_FREE_ENABLED)
            if (this->sample_category_enabled(sample_category::memory)) {
                // run free
                std::string free_output = detail::run_subprocess("free -b");
                free_output = std::regex_replace(free_output, whitespace_replace_reg, " ");
                const std::vector<std::string_view> free_lines = detail::split(detail::trim(free_output), '\n');
                assert((free_lines.size() >= 3) && "Must read more than three lines, but fewer were read!");

                // read memory information
                const std::vector<std::string_view> memory_data = detail::split(free_lines[1], ' ');
                memory_samples_.memory_used_->push_back(detail::convert_to<decltype(memory_samples_.memory_used_)::value_type::value_type>(memory_data[2]));
                memory_samples_.memory_free_->push_back(detail::convert_to<decltype(memory_samples_.memory_free_)::value_type::value_type>(memory_data[3]));

                // read swap information
                const std::vector<std::string_view> swap_data = detail::split(free_lines[2], ' ');
                memory_samples_.swap_memory_used_->push_back(detail::convert_to<decltype(memory_samples_.swap_memory_used_)::value_type::value_type>(swap_data[2]));
                memory_samples_.swap_memory_free_->push_back(detail::convert_to<decltype(memory_samples_.swap_memory_free_)::value_type::value_type>(swap_data[3]));
            }
#endif

#if defined(HWS_VIA_TURBOSTAT_ENABLED)
            {
                // run turbostat
                const std::string turbostat_output = detail::run_subprocess(turbostat_command_line);

                // retrieve the turbostat data
                const std::vector<std::string_view> data = detail::split(detail::trim(turbostat_output), '\n');
                assert((data.size() >= 2) && "Must read at least two lines!");
                const std::vector<std::string_view> header = detail::split(data[0], '\t');
                const std::vector<std::string_view> values = detail::split(data[1], '\t');

                // add values to the respective sample entries
                for (std::size_t i = 0; i < header.size(); ++i) {
                    // general samples
                    if (header[i] == "Busy%") {
                        if (this->sample_category_enabled(sample_category::general)) {
                            using vector_type = decltype(general_samples_.compute_utilization_)::value_type;
                            general_samples_.compute_utilization_->push_back(detail::convert_to<typename vector_type::value_type>(values[i]));
                        }
                        continue;
                    } else if (header[i] == "IPC") {
                        if (this->sample_category_enabled(sample_category::general)) {
                            using vector_type = decltype(general_samples_.ipc_)::value_type;
                            general_samples_.ipc_->push_back(detail::convert_to<typename vector_type::value_type>(values[i]));
                        }
                        continue;
                    } else if (header[i] == "IRQ") {
                        if (this->sample_category_enabled(sample_category::general)) {
                            using vector_type = decltype(general_samples_.irq_)::value_type;
                            general_samples_.irq_->push_back(detail::convert_to<typename vector_type::value_type>(values[i]));
                        }
                        continue;
                    } else if (header[i] == "SMI") {
                        if (this->sample_category_enabled(sample_category::general)) {
                            using vector_type = decltype(general_samples_.smi_)::value_type;
                            general_samples_.smi_->push_back(detail::convert_to<typename vector_type::value_type>(values[i]));
                        }
                        continue;
                    } else if (header[i] == "POLL") {
                        if (this->sample_category_enabled(sample_category::general)) {
                            using vector_type = decltype(general_samples_.poll_)::value_type;
                            general_samples_.poll_->push_back(detail::convert_to<typename vector_type::value_type>(values[i]));
                        }
                        continue;
                    } else if (header[i] == "POLL%") {
                        if (this->sample_category_enabled(sample_category::general)) {
                            using vector_type = decltype(general_samples_.poll_percent_)::value_type;
                            general_samples_.poll_percent_->push_back(detail::convert_to<typename vector_type::value_type>(values[i]));
                        }
                        continue;
                    }

                    // clock related samples
                    if (header[i] == "Avg_MHz") {
                        if (this->sample_category_enabled(sample_category::clock)) {
                            using vector_type = decltype(clock_samples_.clock_frequency_)::value_type;
                            clock_samples_.clock_frequency_->push_back(detail::convert_to<typename vector_type::value_type>(values[i]));
                        }
                        continue;
                    } else if (header[i] == "Bzy_MHz") {
                        if (this->sample_category_enabled(sample_category::clock)) {
                            using vector_type = decltype(clock_samples_.average_non_idle_clock_frequency_)::value_type;
                            clock_samples_.average_non_idle_clock_frequency_->push_back(detail::convert_to<typename vector_type::value_type>(values[i]));
                        }
                        continue;
                    } else if (header[i] == "TSC_MHz") {
                        if (this->sample_category_enabled(sample_category::clock)) {
                            using vector_type = decltype(clock_samples_.time_stamp_counter_)::value_type;
                            clock_samples_.time_stamp_counter_->push_back(detail::convert_to<typename vector_type::value_type>(values[i]));
                        }
                        continue;
                    }

                    // power related samples
                    if (header[i] == "PkgWatt") {
                        if (this->sample_category_enabled(sample_category::power)) {
                            using vector_type = decltype(power_samples_.power_usage_)::value_type;
                            power_samples_.power_usage_->push_back(detail::convert_to<typename vector_type::value_type>(values[i]));
                            // calculate total energy consumption
                            using value_type = decltype(power_samples_.power_total_energy_consumption_)::value_type::value_type;
                            const std::size_t num_time_points = this->sampling_time_points().size();
                            const value_type time_difference = std::chrono::duration<value_type>(this->sampling_time_points()[num_time_points - 1] - this->sampling_time_points()[num_time_points - 2]).count();
                            const auto current = power_samples_.power_usage_->back() * time_difference;
                            power_samples_.power_total_energy_consumption_->push_back(power_samples_.power_total_energy_consumption_->back() + current);
                        }
                        continue;
                    } else if (header[i] == "CorWatt") {
                        if (this->sample_category_enabled(sample_category::power)) {
                            using vector_type = decltype(power_samples_.core_watt_)::value_type;
                            power_samples_.core_watt_->push_back(detail::convert_to<typename vector_type::value_type>(values[i]));
                        }
                        continue;
                    } else if (header[i] == "RAMWatt") {
                        if (this->sample_category_enabled(sample_category::power)) {
                            using vector_type = decltype(power_samples_.ram_watt_)::value_type;
                            power_samples_.ram_watt_->push_back(detail::convert_to<typename vector_type::value_type>(values[i]));
                        }
                        continue;
                    } else if (header[i] == "PKG_%") {
                        if (this->sample_category_enabled(sample_category::power)) {
                            using vector_type = decltype(power_samples_.package_rapl_throttle_percent_)::value_type;
                            power_samples_.package_rapl_throttle_percent_->push_back(detail::convert_to<typename vector_type::value_type>(values[i]));
                        }
                        continue;
                    } else if (header[i] == "RAM_%") {
                        if (this->sample_category_enabled(sample_category::power)) {
                            using vector_type = decltype(power_samples_.dram_rapl_throttle_percent_)::value_type;
                            power_samples_.dram_rapl_throttle_percent_->push_back(detail::convert_to<typename vector_type::value_type>(values[i]));
                        }
                        continue;
                    }

                    // temperature related samples
                    if (header[i] == "CoreTmp") {
                        if (this->sample_category_enabled(sample_category::temperature)) {
                            using vector_type = decltype(temperature_samples_.core_temperature_)::value_type;
                            temperature_samples_.core_temperature_->push_back(detail::convert_to<typename vector_type::value_type>(values[i]));
                        }
                        continue;
                    } else if (header[i] == "CoreThr") {
                        if (this->sample_category_enabled(sample_category::temperature)) {
                            using vector_type = decltype(temperature_samples_.core_throttle_percent_)::value_type;
                            temperature_samples_.core_throttle_percent_->push_back(detail::convert_to<typename vector_type::value_type>(values[i]));
                        }
                        continue;
                    } else if (header[i] == "PkgTmp") {
                        if (this->sample_category_enabled(sample_category::temperature)) {
                            using vector_type = decltype(temperature_samples_.temperature_)::value_type;
                            temperature_samples_.temperature_->push_back(detail::convert_to<typename vector_type::value_type>(values[i]));
                        }
                        continue;
                    }

                    // gfx (iGPU) related samples
                    if (header[i] == "GFX%rc6") {
                        if (this->sample_category_enabled(sample_category::gfx)) {
                            using vector_type = decltype(gfx_samples_.gfx_render_state_percent_)::value_type;
                            gfx_samples_.gfx_render_state_percent_->push_back(detail::convert_to<typename vector_type::value_type>(values[i]));
                        }
                        continue;
                    } else if (header[i] == "GFXMHz") {
                        if (this->sample_category_enabled(sample_category::gfx)) {
                            using vector_type = decltype(gfx_samples_.gfx_frequency_)::value_type;
                            gfx_samples_.gfx_frequency_->push_back(detail::convert_to<typename vector_type::value_type>(values[i]));
                        }
                        continue;
                    } else if (header[i] == "GFXAMHz") {
                        if (this->sample_category_enabled(sample_category::gfx)) {
                            using vector_type = decltype(gfx_samples_.average_gfx_frequency_)::value_type;
                            gfx_samples_.average_gfx_frequency_->push_back(detail::convert_to<typename vector_type::value_type>(values[i]));
                        }
                        continue;
                    } else if (header[i] == "GFX%C0") {
                        if (this->sample_category_enabled(sample_category::gfx)) {
                            using vector_type = decltype(gfx_samples_.gfx_state_c0_percent_)::value_type;
                            gfx_samples_.gfx_state_c0_percent_->push_back(detail::convert_to<typename vector_type::value_type>(values[i]));
                        }
                        continue;
                    } else if (header[i] == "CPUGFX%") {
                        if (this->sample_category_enabled(sample_category::gfx)) {
                            using vector_type = decltype(gfx_samples_.cpu_works_for_gpu_percent_)::value_type;
                            gfx_samples_.cpu_works_for_gpu_percent_->push_back(detail::convert_to<typename vector_type::value_type>(values[i]));
                        }
                        continue;
                    } else if (header[i] == "GFXWatt") {
                        if (this->sample_category_enabled(sample_category::gfx)) {
                            using vector_type = decltype(gfx_samples_.gfx_watt_)::value_type;
                            gfx_samples_.gfx_watt_->push_back(detail::convert_to<typename vector_type::value_type>(values[i]));
                        }
                        continue;
                    }

                    // idle state related samples
                    if (header[i] == "Totl%C0") {
                        if (this->sample_category_enabled(sample_category::idle_state)) {
                            using vector_type = decltype(idle_state_samples_.all_cpus_state_c0_percent_)::value_type;
                            idle_state_samples_.all_cpus_state_c0_percent_->push_back(detail::convert_to<typename vector_type::value_type>(values[i]));
                        }
                        continue;
                    } else if (header[i] == "Any%C0") {
                        if (this->sample_category_enabled(sample_category::idle_state)) {
                            using vector_type = decltype(idle_state_samples_.any_cpu_state_c0_percent_)::value_type;
                            idle_state_samples_.any_cpu_state_c0_percent_->push_back(detail::convert_to<typename vector_type::value_type>(values[i]));
                        }
                        continue;
                    } else if (header[i] == "CPU%LPI") {
                        if (this->sample_category_enabled(sample_category::idle_state)) {
                            using vector_type = decltype(idle_state_samples_.low_power_idle_state_percent_)::value_type;
                            idle_state_samples_.low_power_idle_state_percent_->push_back(detail::convert_to<typename vector_type::value_type>(values[i]));
                        }
                        continue;
                    } else if (header[i] == "SYS%LPI") {
                        if (this->sample_category_enabled(sample_category::idle_state)) {
                            using vector_type = decltype(idle_state_samples_.system_low_power_idle_state_percent_)::value_type;
                            idle_state_samples_.system_low_power_idle_state_percent_->push_back(detail::convert_to<typename vector_type::value_type>(values[i]));
                        }
                        continue;
                    } else if (header[i] == "Pkg%LPI") {
                        if (this->sample_category_enabled(sample_category::idle_state)) {
                            using vector_type = decltype(idle_state_samples_.package_low_power_idle_state_percent_)::value_type;
                            idle_state_samples_.package_low_power_idle_state_percent_->push_back(detail::convert_to<typename vector_type::value_type>(values[i]));
                        }
                        continue;
                    } else {
                        if (this->sample_category_enabled(sample_category::idle_state)) {
                            const std::string header_str{ header[i] };
                            if (idle_state_samples_.idle_states_.has_value() && idle_state_samples_.idle_states_.value().count(header_str) > decltype(idle_state_samples_)::map_type::size_type{ 0 }) {
                                using vector_type = cpu_idle_states_samples::map_type::mapped_type;
                                idle_state_samples_.idle_states_.value()[header_str].push_back(detail::convert_to<typename vector_type::value_type>(values[i]));
                            }
                        }
                        continue;
                    }
                }
            }
#endif
        }

        // wait for the sampling interval to pass to retrieve the next sample
        std::this_thread::sleep_for(this->sampling_interval());
    }
}

std::string cpu_hardware_sampler::device_identification() const {
    return "cpu_device";
}

std::string cpu_hardware_sampler::samples_only_as_yaml_string() const {
    // check whether it's safe to generate the YAML entry
    if (this->is_sampling()) {
        throw std::runtime_error{ "Can't create the final YAML entry if the hardware sampler is still running!" };
    }

    return fmt::format("{}{}"
                       "{}{}"
                       "{}{}"
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
                       temperature_samples_.generate_yaml_string(),
                       temperature_samples_.has_samples() ? "\n" : "",
                       gfx_samples_.generate_yaml_string(),
                       gfx_samples_.has_samples() ? "\n" : "",
                       idle_state_samples_.generate_yaml_string());
}

std::ostream &operator<<(std::ostream &out, const cpu_hardware_sampler &sampler) {
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
                                  "temperature samples:\n{}\n\n"
                                  "gfx samples:\n{}\n\n"
                                  "idle state samples:\n{}",
                                  sampler.sampling_interval(),
                                  fmt::join(detail::time_points_to_epoch(sampler.sampling_time_points()), ", "),
                                  sampler.general_samples(),
                                  sampler.clock_samples(),
                                  sampler.power_samples(),
                                  sampler.memory_samples(),
                                  sampler.temperature_samples(),
                                  sampler.gfx_samples(),
                                  sampler.idle_state_samples());
    }
}

}  // namespace hws
