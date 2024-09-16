/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hardware_sampling/gpu_intel/hardware_sampler.hpp"    // hws::gpu_intel_hardware_sampler
#include "hardware_sampling/gpu_intel/level_zero_samples.hpp"  // hws::{level_zero_general_samples, level_zero_clock_samples, level_zero_power_samples, level_zero_memory_samples, level_zero_temperature_samples}
#include "hardware_sampling/hardware_sampler.hpp"              // hws::hardware_sampler

#include "fmt/format.h"         // fmt::format
#include "pybind11/chrono.h"    // automatic bindings for std::chrono::milliseconds
#include "pybind11/pybind11.h"  // py::module_
#include "pybind11/stl.h"       // bind STL types

#include <chrono>   // std::chrono::milliseconds
#include <cstddef>  // std::size_t

namespace py = pybind11;

void init_gpu_intel_hardware_sampler(py::module_ &m) {
    // bind the general samples
    py::class_<hws::level_zero_general_samples>(m, "LevelZeroGeneralSamples")
        .def("get_name", &hws::level_zero_general_samples::get_name, "the model name of the device")
        .def("get_standby_mode", &hws::level_zero_general_samples::get_standby_mode, "the enabled standby mode (power saving or never)")
        .def("get_num_threads_per_eu", &hws::level_zero_general_samples::get_num_threads_per_eu, "the number of threads per EU unit")
        .def("get_eu_simd_width", &hws::level_zero_general_samples::get_eu_simd_width, "the physical EU unit SIMD width")
        .def("__repr__", [](const hws::level_zero_general_samples &self) {
            return fmt::format("<HardwareSampling.LevelZeroGeneralSamples with\n{}\n>", self);
        });

    // bind the clock samples
    py::class_<hws::level_zero_clock_samples>(m, "LevelZeroClockSamples")
        .def("get_clock_gpu_min", &hws::level_zero_clock_samples::get_clock_gpu_min, "the minimum possible GPU clock frequency in MHz")
        .def("get_clock_gpu_max", &hws::level_zero_clock_samples::get_clock_gpu_max, "the maximum possible GPU clock frequency in MHz")
        .def("get_available_clocks_gpu", &hws::level_zero_clock_samples::get_available_clocks_gpu, "the available GPU clock frequencies in MHz (slowest to fastest)")
        .def("get_clock_mem_min", &hws::level_zero_clock_samples::get_clock_mem_min, "the minimum possible memory clock frequency in MHz")
        .def("get_clock_mem_max", &hws::level_zero_clock_samples::get_clock_mem_max, "the maximum possible memory clock frequency in MHz")
        .def("get_available_clocks_mem", &hws::level_zero_clock_samples::get_available_clocks_mem, "the available memory clock frequencies in MHz (slowest to fastest)")
        .def("get_tdp_frequency_limit_gpu", &hws::level_zero_clock_samples::get_tdp_frequency_limit_gpu, "the current maximum allowed GPU frequency based on the TDP limit in MHz")
        .def("get_clock_gpu", &hws::level_zero_clock_samples::get_clock_gpu, "the current GPU frequency in MHz")
        .def("get_throttle_reason_gpu", &hws::level_zero_clock_samples::get_throttle_reason_gpu, "the current GPU frequency throttle reason")
        .def("get_tdp_frequency_limit_mem", &hws::level_zero_clock_samples::get_tdp_frequency_limit_mem, "the current maximum allowed memory frequency based on the TDP limit in MHz")
        .def("get_clock_mem", &hws::level_zero_clock_samples::get_clock_mem, "the current memory frequency in MHz")
        .def("get_throttle_reason_mem", &hws::level_zero_clock_samples::get_throttle_reason_mem, "the current memory frequency throttle reason")
        .def("__repr__", [](const hws::level_zero_clock_samples &self) {
            return fmt::format("<HardwareSampling.LevelZeroClockSamples with\n{}\n>", self);
        });

    // bind the power samples
    py::class_<hws::level_zero_power_samples>(m, "LevelZeroPowerSamples")
        .def("get_energy_threshold_enabled", &hws::level_zero_power_samples::get_energy_threshold_enabled, "true if the energy threshold is enabled")
        .def("get_energy_threshold", &hws::level_zero_power_samples::get_energy_threshold, "the energy threshold in J")
        .def("get_power_total_energy_consumption", &hws::level_zero_power_samples::get_power_total_energy_consumption, "the total power consumption since the last driver reload in mJ")
        .def("__repr__", [](const hws::level_zero_power_samples &self) {
            return fmt::format("<HardwareSampling.LevelZeroPowerSamples with\n{}\n>", self);
        });

    // bind the memory samples
    py::class_<hws::level_zero_memory_samples>(m, "LevelZeroMemorySamples")
        .def("get_memory_total", &hws::level_zero_memory_samples::get_memory_total, "the total memory size of the different memory modules in Bytes")
        .def("get_allocatable_memory_total", &hws::level_zero_memory_samples::get_allocatable_memory_total, "the total allocatable memory size of the different memory modules in Bytes")
        .def("get_pcie_link_max_speed", &hws::level_zero_memory_samples::get_pcie_link_max_speed, "the maximum PCIe bandwidth in bytes/sec")
        .def("get_pcie_max_width", &hws::level_zero_memory_samples::get_pcie_max_width, "the PCIe lane width")
        .def("get_max_pcie_link_generation", &hws::level_zero_memory_samples::get_max_pcie_link_generation, "the PCIe generation")
        .def("get_bus_width", &hws::level_zero_memory_samples::get_bus_width, "the bus width of the different memory modules")
        .def("get_num_channels", &hws::level_zero_memory_samples::get_num_channels, "the number of memory channels of the different memory modules")
        .def("get_location", &hws::level_zero_memory_samples::get_location, "the location of the different memory modules (system or device)")
        .def("get_memory_free", &hws::level_zero_memory_samples::get_memory_free, "the currently free memory of the different memory modules in Bytes")
        .def("get_pcie_link_speed", &hws::level_zero_memory_samples::get_pcie_link_speed, "the current PCIe bandwidth in bytes/sec")
        .def("get_pcie_link_width", &hws::level_zero_memory_samples::get_pcie_link_width, "the current PCIe lane width")
        .def("get_pcie_link_generation", &hws::level_zero_memory_samples::get_pcie_link_generation, "the current PCIe generation")
        .def("__repr__", [](const hws::level_zero_memory_samples &self) {
            return fmt::format("<HardwareSampling.LevelZeroMemorySamples with\n{}\n>", self);
        });

    // bind the temperature samples
    py::class_<hws::level_zero_temperature_samples>(m, "LevelZeroTemperatureSamples")
        .def("get_temperature_max", &hws::level_zero_temperature_samples::get_temperature_max, "the maximum temperature for the sensor in °C")
        .def("get_temperature_psu", &hws::level_zero_temperature_samples::get_temperature_psu, "the temperature of the PSU in °C")
        .def("get_temperature", &hws::level_zero_temperature_samples::get_temperature, "the current temperature for the sensor in °C")
        .def("__repr__", [](const hws::level_zero_temperature_samples &self) {
            return fmt::format("<HardwareSampling.LevelZeroTemperatureSamples with\n{}\n>", self);
        });

    // bind the GPU Intel hardware sampler class
    py::class_<hws::gpu_intel_hardware_sampler, hws::hardware_sampler>(m, "GpuIntelHardwareSampler")
        .def(py::init<>(), "construct a new Intel GPU hardware sampler for the default device with the default sampling interval")
        .def(py::init<std::size_t>(), "construct a new Intel GPU hardware sampler for the specified device with the default sampling interval")
        .def(py::init<std::chrono::milliseconds>(), "construct a new Intel GPU hardware sampler for the default device with the specified sampling interval")
        .def(py::init<std::size_t, std::chrono::milliseconds>(), "construct a new Intel GPU hardware sampler for the specified device and sampling interval")
        .def("general_samples", &hws::gpu_intel_hardware_sampler::general_samples, "get all general samples")
        .def("clock_samples", &hws::gpu_intel_hardware_sampler::clock_samples, "get all clock related samples")
        .def("power_samples", &hws::gpu_intel_hardware_sampler::power_samples, "get all power related samples")
        .def("memory_samples", &hws::gpu_intel_hardware_sampler::memory_samples, "get all memory related samples")
        .def("temperature_samples", &hws::gpu_intel_hardware_sampler::temperature_samples, "get all temperature related samples")
        .def("__repr__", [](const hws::gpu_intel_hardware_sampler &self) {
            return fmt::format("<HardwareSampling.GpuIntelHardwareSampler with\n{}\n>", self);
        });
}
