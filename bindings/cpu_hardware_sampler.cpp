/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hardware_sampling/cpu/cpu_samples.hpp"       // hws::{cpu_general_samples, clock_samples, power_samples, memory_samples, temperature_samples, gfx_samples, idle_state_samples}
#include "hardware_sampling/cpu/hardware_sampler.hpp"  // hws::cpu_hardware_sampler
#include "hardware_sampling/hardware_sampler.hpp"      // hws::hardware_sampler

#include "fmt/format.h"         // fmt::format
#include "pybind11/chrono.h"    // automatic bindings for std::chrono::milliseconds
#include "pybind11/pybind11.h"  // py::module_
#include "pybind11/stl.h"       // bind STL types

#include <chrono>  // std::chrono::milliseconds

namespace py = pybind11;

void init_cpu_hardware_sampler(py::module_ &m) {
    // bind the general samples
    py::class_<hws::cpu_general_samples>(m, "CpuGeneralSamples")
        .def("get_architecture", &hws::cpu_general_samples::get_architecture, "the CPU architecture (e.g., x86_64)")
        .def("get_byte_order", &hws::cpu_general_samples::get_byte_order, "the byte order (e.g., little/big endian)")
        .def("get_num_cores", &hws::cpu_general_samples::get_num_cores, "the total number of cores of the CPU(s)")
        .def("get_num_threads", &hws::cpu_general_samples::get_num_threads, "the number of threads of the CPU(s) including potential hyper-threads")
        .def("get_threads_per_core", &hws::cpu_general_samples::get_threads_per_core, "the number of hyper-threads per core")
        .def("get_cores_per_socket", &hws::cpu_general_samples::get_cores_per_socket, "the number of physical cores per socket")
        .def("get_num_sockets", &hws::cpu_general_samples::get_num_sockets, "the number of sockets")
        .def("get_numa_nodes", &hws::cpu_general_samples::get_numa_nodes, "the number of NUMA nodes")
        .def("get_vendor_id", &hws::cpu_general_samples::get_vendor_id, "the vendor ID (e.g. GenuineIntel)")
        .def("get_name", &hws::cpu_general_samples::get_name, "the name of the CPU")
        .def("get_flags", &hws::cpu_general_samples::get_flags, "potential CPU flags (e.g., sse4_1, avx, avx, etc)")
        .def("get_compute_utilization", &hws::cpu_general_samples::get_compute_utilization, "the percent the CPU was busy doing work")
        .def("get_ipc", &hws::cpu_general_samples::get_ipc, "the instructions-per-cycle count")
        .def("get_irq", &hws::cpu_general_samples::get_irq, "the number of interrupts")
        .def("get_smi", &hws::cpu_general_samples::get_smi, "the number of system management interrupts")
        .def("get_poll", &hws::cpu_general_samples::get_poll, "the number of times the CPU was in the polling state")
        .def("get_poll_percent", &hws::cpu_general_samples::get_poll_percent, "the percent of the CPU was in the polling state")
        .def("__repr__", [](const hws::cpu_general_samples &self) {
            return fmt::format("<HardwareSampling.CpuGeneralSamples with\n{}\n>", self);
        });

    // bind the clock samples
    py::class_<hws::cpu_clock_samples>(m, "CpuClockSamples")
        .def("get_auto_boosted_clock_enabled", &hws::cpu_clock_samples::get_auto_boosted_clock_enabled, "true if frequency boosting is enabled")
        .def("get_clock_frequency_min", &hws::cpu_clock_samples::get_clock_frequency_min, "the minimum possible CPU frequency in MHz")
        .def("get_clock_frequency_max", &hws::cpu_clock_samples::get_clock_frequency_max, "the maximum possible CPU frequency in MHz")
        .def("get_clock_frequency", &hws::cpu_clock_samples::get_clock_frequency, "the average CPU frequency in MHz including idle cores")
        .def("get_average_non_idle_clock_frequency", &hws::cpu_clock_samples::get_average_non_idle_clock_frequency, "the average CPU frequency in MHz excluding idle cores")
        .def("get_time_stamp_counter", &hws::cpu_clock_samples::get_time_stamp_counter, "the time stamp counter")
        .def("__repr__", [](const hws::cpu_clock_samples &self) {
            return fmt::format("<HardwareSampling.CpuClockSamples with\n{}\n>", self);
        });

    // bind the power samples
    py::class_<hws::cpu_power_samples>(m, "CpuPowerSamples")
        .def("get_power_measurement_type", &hws::cpu_power_samples::get_power_measurement_type, "the type of the power readings: always \"instant/current\"")
        .def("get_power_usage", &hws::cpu_power_samples::get_power_usage, "the currently consumed power of the package of the CPU in W")
        .def("get_power_total_energy_consumed", &hws::cpu_power_samples::get_power_total_energy_consumption, "the total power consumption in J")
        .def("get_core_watt", &hws::cpu_power_samples::get_core_watt, "the currently consumed power of the core part of the CPU in W")
        .def("get_ram_watt", &hws::cpu_power_samples::get_ram_watt, "the currently consumed power of the RAM part of the CPU in W")
        .def("get_package_rapl_throttle_percent", &hws::cpu_power_samples::get_package_rapl_throttle_percent, "the percent of time the package throttled due to RAPL limiters")
        .def("get_dram_rapl_throttle_percent", &hws::cpu_power_samples::get_dram_rapl_throttle_percent, "the percent of time the DRAM throttled due to RAPL limiters")
        .def("__repr__", [](const hws::cpu_power_samples &self) {
            return fmt::format("<HardwareSampling.CpuPowerSamples with\n{}\n>", self);
        });

    // bind the memory samples
    py::class_<hws::cpu_memory_samples>(m, "CpuMemorySamples")
        .def("get_cache_size_L1d", &hws::cpu_memory_samples::get_cache_size_L1d, "the size of the L1 data cache")
        .def("get_cache_size_L1i", &hws::cpu_memory_samples::get_cache_size_L1i, "the size of the L1 instruction cache")
        .def("get_cache_size_L2", &hws::cpu_memory_samples::get_cache_size_L2, "the size of the L2 cache")
        .def("get_cache_size_L3", &hws::cpu_memory_samples::get_cache_size_L3, "the size of the L2 cache")
        .def("get_memory_total", &hws::cpu_memory_samples::get_memory_total, "the total available memory in Byte")
        .def("get_swap_memory_total", &hws::cpu_memory_samples::get_swap_memory_total, "the total available swap memory in Byte")
        .def("get_memory_used", &hws::cpu_memory_samples::get_memory_used, "the currently used memory in Byte")
        .def("get_memory_free", &hws::cpu_memory_samples::get_memory_free, "the currently free memory in Byte")
        .def("get_swap_memory_used", &hws::cpu_memory_samples::get_swap_memory_used, "the currently used swap memory in Byte")
        .def("get_swap_memory_free", &hws::cpu_memory_samples::get_swap_memory_free, "the currently free swap memory in Byte")
        .def("__repr__", [](const hws::cpu_memory_samples &self) {
            return fmt::format("<HardwareSampling.CpuMemorySamples with\n{}\n>", self);
        });

    // bind the temperature samples
    py::class_<hws::cpu_temperature_samples>(m, "CpuTemperatureSamples")
        .def("get_temperature", &hws::cpu_temperature_samples::get_temperature, "the current temperature of the whole package in °C")
        .def("get_core_temperature", &hws::cpu_temperature_samples::get_core_temperature, "the current temperature of the core part of the CPU in °C")
        .def("get_core_throttle_percent", &hws::cpu_temperature_samples::get_core_throttle_percent, "the percent of time the CPU has throttled")
        .def("__repr__", [](const hws::cpu_temperature_samples &self) {
            return fmt::format("<HardwareSampling.CpuTemperatureSamples with\n{}\n>", self);
        });

    // bind the gfx samples
    py::class_<hws::cpu_gfx_samples>(m, "CpuGfxSamples")
        .def("get_gfx_render_state_percent", &hws::cpu_gfx_samples::get_gfx_render_state_percent, "the percent of time the iGPU was in the render state")
        .def("get_gfx_frequency", &hws::cpu_gfx_samples::get_gfx_frequency, "the current iGPU power consumption in W")
        .def("get_average_gfx_frequency", &hws::cpu_gfx_samples::get_average_gfx_frequency, "the average iGPU frequency in MHz")
        .def("get_gfx_state_c0_percent", &hws::cpu_gfx_samples::get_gfx_state_c0_percent, "the percent of the time the iGPU was in the c0 state")
        .def("get_cpu_works_for_gpu_percent", &hws::cpu_gfx_samples::get_cpu_works_for_gpu_percent, "the percent of time the CPU was doing work for the iGPU")
        .def("get_gfx_watt", &hws::cpu_gfx_samples::get_gfx_watt, "the currently consumed power of the iGPU of the CPU in W")
        .def("__repr__", [](const hws::cpu_gfx_samples &self) {
            return fmt::format("<HardwareSampling.CpuGfxSamples with\n{}\n>", self);
        });

    // bind the idle state samples
    py::class_<hws::cpu_idle_states_samples>(m, "CpuIdleStateSamples")
        .def("get_idle_states", &hws::cpu_idle_states_samples::get_idle_states, "the map of additional CPU idle states")
        .def("get_all_cpus_state_c0_percent", &hws::cpu_idle_states_samples::get_all_cpus_state_c0_percent, "the percent of time all CPUs were in idle state c0")
        .def("get_any_cpu_state_c0_percent", &hws::cpu_idle_states_samples::get_any_cpu_state_c0_percent, "the percent of time any CPU was in the idle state c0")
        .def("get_low_power_idle_state_percent", &hws::cpu_idle_states_samples::get_low_power_idle_state_percent, "the percent of time the CPUs was in the low power idle state")
        .def("get_system_low_power_idle_state_percent", &hws::cpu_idle_states_samples::get_system_low_power_idle_state_percent, "the percent of time the CPU was in the system low power idle state")
        .def("get_package_low_power_idle_state_percent", &hws::cpu_idle_states_samples::get_package_low_power_idle_state_percent, "the percent of time the CPU was in the package low power idle state")
        .def("__repr__", [](const hws::cpu_gfx_samples &self) {
            return fmt::format("<HardwareSampling.CpuIdleStateSamples with\n{}\n>", self);
        });

    // bind the CPU hardware sampler class
    py::class_<hws::cpu_hardware_sampler, hws::hardware_sampler>(m, "CpuHardwareSampler")
        .def(py::init<>(), "construct a new CPU hardware sampler")
        .def(py::init<std::chrono::milliseconds>(), "construct a new CPU hardware sampler specifying the used sampling interval")
        .def("general_samples", &hws::cpu_hardware_sampler::general_samples, "get all general samples")
        .def("clock_samples", &hws::cpu_hardware_sampler::clock_samples, "get all clock related samples")
        .def("power_samples", &hws::cpu_hardware_sampler::power_samples, "get all power related samples")
        .def("memory_samples", &hws::cpu_hardware_sampler::memory_samples, "get all memory related samples")
        .def("temperature_samples", &hws::cpu_hardware_sampler::temperature_samples, "get all temperature related samples")
        .def("gfx_samples", &hws::cpu_hardware_sampler::gfx_samples, "get all gfx (iGPU) related samples")
        .def("idle_state_samples", &hws::cpu_hardware_sampler::idle_state_samples, "get all idle state related samples")
        .def("__repr__", [](const hws::cpu_hardware_sampler &self) {
            return fmt::format("<HardwareSampling.CpuHardwareSampler with\n{}\n>", self);
        });
}
