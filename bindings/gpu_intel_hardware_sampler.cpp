/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hardware_sampling/gpu_intel/hardware_sampler.hpp"    // hws::gpu_intel_hardware_sampler
#include "hardware_sampling/gpu_intel/level_zero_samples.hpp"  // hws::{level_zero_general_samples, level_zero_clock_samples, level_zero_power_samples, level_zero_memory_samples, level_zero_temperature_samples}
#include "hardware_sampling/hardware_sampler.hpp"              // hws::hardware_sampler
#include "hardware_sampling/sample_category.hpp"               // hws::sample_category

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
        .def("has_samples", &hws::level_zero_general_samples::has_samples, "true if any sample is available, false otherwise")
        .def("get_byte_order", &hws::level_zero_general_samples::get_byte_order, "the byte order (e.g., little/big endian)")
        .def("get_vendor_id", &hws::level_zero_general_samples::get_vendor_id, "the vendor ID")
        .def("get_name", &hws::level_zero_general_samples::get_name, "the model name of the device")
        .def("get_flags", &hws::level_zero_general_samples::get_flags, "potential GPU flags (e.g. integrated device)")
        .def("get_standby_mode", &hws::level_zero_general_samples::get_standby_mode, "the enabled standby mode (power saving or never)")
        .def("get_num_threads_per_eu", &hws::level_zero_general_samples::get_num_threads_per_eu, "the number of threads per EU unit")
        .def("get_eu_simd_width", &hws::level_zero_general_samples::get_eu_simd_width, "the physical EU unit SIMD width")
        .def("__repr__", [](const hws::level_zero_general_samples &self) {
            return fmt::format("<HardwareSampling.LevelZeroGeneralSamples with\n{}\n>", self);
        });

    // bind the clock samples
    py::class_<hws::level_zero_clock_samples>(m, "LevelZeroClockSamples")
        .def("has_samples", &hws::level_zero_clock_samples::has_samples, "true if any sample is available, false otherwise")
        .def("get_clock_frequency_min", &hws::level_zero_clock_samples::get_clock_frequency_min, "the minimum possible GPU clock frequency in MHz")
        .def("get_clock_frequency_max", &hws::level_zero_clock_samples::get_clock_frequency_max, "the maximum possible GPU clock frequency in MHz")
        .def("get_memory_clock_frequency_min", &hws::level_zero_clock_samples::get_memory_clock_frequency_min, "the minimum possible memory clock frequency in MHz")
        .def("get_memory_clock_frequency_max", &hws::level_zero_clock_samples::get_memory_clock_frequency_max, "the maximum possible memory clock frequency in MHz")
        .def("get_available_clock_frequencies", &hws::level_zero_clock_samples::get_available_clock_frequencies, "the available GPU clock frequencies in MHz (slowest to fastest)")
        .def("get_available_memory_clock_frequencies", &hws::level_zero_clock_samples::get_available_memory_clock_frequencies, "the available memory clock frequencies in MHz (slowest to fastest)")
        .def("get_clock_frequency", &hws::level_zero_clock_samples::get_clock_frequency, "the current GPU frequency in MHz")
        .def("get_memory_clock_frequency", &hws::level_zero_clock_samples::get_memory_clock_frequency, "the current memory frequency in MHz")
        .def("get_throttle_reason", &hws::level_zero_clock_samples::get_throttle_reason, "the current GPU frequency throttle reason (as bitmask)")
        .def("get_throttle_reason_string", &hws::level_zero_clock_samples::get_throttle_reason_string, "the current GPU frequency throttle reason (as string)")
        .def("get_memory_throttle_reason", &hws::level_zero_clock_samples::get_memory_throttle_reason, "the current memory frequency throttle reason (as bitmask)")
        .def("get_memory_throttle_reason_string", &hws::level_zero_clock_samples::get_memory_throttle_reason_string, "the current memory frequency throttle reason (as string)")
        .def("get_frequency_limit_tdp", &hws::level_zero_clock_samples::get_frequency_limit_tdp, "the current maximum allowed GPU frequency based on the TDP limit in MHz")
        .def("get_memory_frequency_limit_tdp", &hws::level_zero_clock_samples::get_memory_frequency_limit_tdp, "the current maximum allowed memory frequency based on the TDP limit in MHz")
        .def("__repr__", [](const hws::level_zero_clock_samples &self) {
            return fmt::format("<HardwareSampling.LevelZeroClockSamples with\n{}\n>", self);
        });

    // bind the power samples
    py::class_<hws::level_zero_power_samples>(m, "LevelZeroPowerSamples")
        .def("has_samples", &hws::level_zero_power_samples::has_samples, "true if any sample is available, false otherwise")
        .def("get_power_enforced_limit", &hws::level_zero_power_samples::get_power_enforced_limit, "the actually enforced power limit (W), may be different from power management limit if external limiters are set")
        .def("get_power_measurement_type", &hws::level_zero_power_samples::get_power_measurement_type, "the type of the power readings")
        .def("get_power_management_mode", &hws::level_zero_power_samples::get_power_management_mode, "true if power management limits are enabled")
        .def("get_power_usage", &hws::level_zero_power_samples::get_power_usage, "the current power draw of the GPU in W (calculated from power_total_energy_consumption)")
        .def("get_power_total_energy_consumption", &hws::level_zero_power_samples::get_power_total_energy_consumption, "the total power consumption since the last driver reload in J")
        .def("__repr__", [](const hws::level_zero_power_samples &self) {
            return fmt::format("<HardwareSampling.LevelZeroPowerSamples with\n{}\n>", self);
        });

    // bind the memory samples
    py::class_<hws::level_zero_memory_samples>(m, "LevelZeroMemorySamples")
        .def("has_samples", &hws::level_zero_memory_samples::has_samples, "true if any sample is available, false otherwise")
        .def("get_memory_total", &hws::level_zero_memory_samples::get_memory_total, "the total memory size of the different memory modules in Bytes")
        .def("get_visible_memory_total", &hws::level_zero_memory_samples::get_visible_memory_total, "the total allocatable memory size of the different memory modules in Bytes")
        .def("get_memory_location", &hws::level_zero_memory_samples::get_memory_location, "the location of the different memory modules (system or device)")
        .def("get_num_pcie_lanes_max", &hws::level_zero_memory_samples::get_num_pcie_lanes_max, "the PCIe lane width")
        .def("get_pcie_link_generation_max", &hws::level_zero_memory_samples::get_pcie_link_generation_max, "the PCIe generation")
        .def("get_pcie_link_speed_max", &hws::level_zero_memory_samples::get_pcie_link_speed_max, "the maximum PCIe bandwidth in bytes/sec")
        .def("get_memory_bus_width", &hws::level_zero_memory_samples::get_memory_bus_width, "the bus width of the different memory modules")
        .def("get_memory_num_channels", &hws::level_zero_memory_samples::get_memory_num_channels, "the number of memory channels of the different memory modules")
        .def("get_memory_free", &hws::level_zero_memory_samples::get_memory_free, "the currently free memory of the different memory modules in Bytes")
        .def("get_memory_used", &hws::level_zero_memory_samples::get_memory_used, "the currently used memory of the different memory modules in Bytes")
        .def("get_num_pcie_lanes", &hws::level_zero_memory_samples::get_num_pcie_lanes, "the current PCIe lane width")
        .def("get_pcie_link_generation", &hws::level_zero_memory_samples::get_pcie_link_generation, "the current PCIe generation")
        .def("get_pcie_link_speed", &hws::level_zero_memory_samples::get_pcie_link_speed, "the current PCIe bandwidth in bytes/sec")
        .def("__repr__", [](const hws::level_zero_memory_samples &self) {
            return fmt::format("<HardwareSampling.LevelZeroMemorySamples with\n{}\n>", self);
        });

    // bind the temperature samples
    py::class_<hws::level_zero_temperature_samples>(m, "LevelZeroTemperatureSamples")
        .def("has_samples", &hws::level_zero_temperature_samples::has_samples, "true if any sample is available, false otherwise")
        .def("get_num_fans", &hws::level_zero_temperature_samples::get_num_fans, "the number of fans")
        .def("get_fan_speed_max", &hws::level_zero_temperature_samples::get_fan_speed_max, "the maximum fan speed the user can set in RPM")
        .def("get_temperature_max", &hws::level_zero_temperature_samples::get_temperature_max, "the maximum GPU temperature in °C")
        .def("get_memory_temperature_max", &hws::level_zero_temperature_samples::get_memory_temperature_max, "the maximum memory temperature in °C")
        .def("get_global_temperature_max", &hws::level_zero_temperature_samples::get_global_temperature_max, "the maximum global temperature in °C")
        .def("get_fan_speed_percentage", &hws::level_zero_temperature_samples::get_fan_speed_percentage, "the current intended fan speed in %")
        .def("get_temperature", &hws::level_zero_temperature_samples::get_temperature, "the current GPU temperature in °C")
        .def("get_memory_temperature", &hws::level_zero_temperature_samples::get_memory_temperature, "the current memory temperature in °C")
        .def("get_global_temperature", &hws::level_zero_temperature_samples::get_global_temperature, "the current global temperature in °C")
        .def("get_psu_temperature", &hws::level_zero_temperature_samples::get_psu_temperature, "the current PSU temperature in °C")
        .def("__repr__", [](const hws::level_zero_temperature_samples &self) {
            return fmt::format("<HardwareSampling.LevelZeroTemperatureSamples with\n{}\n>", self);
        });

    // bind the GPU Intel hardware sampler class
    py::class_<hws::gpu_intel_hardware_sampler, hws::hardware_sampler>(m, "GpuIntelHardwareSampler")
        .def(py::init<>(), "construct a new Intel GPU hardware sampler for the default device with the default sampling interval")
        .def(py::init<hws::sample_category>(), "construct a new Intel GPU hardware sampler for the default device with the default sampling interval sampling only the provided sample_category samples")
        .def(py::init<std::size_t>(), "construct a new Intel GPU hardware sampler for the specified device with the default sampling interval")
        .def(py::init<std::size_t, hws::sample_category>(), "construct a new Intel GPU hardware sampler for the specified device with the default sampling interval sampling only the provided sample_category samples")
        .def(py::init<std::chrono::milliseconds>(), "construct a new Intel GPU hardware sampler for the default device with the specified sampling interval")
        .def(py::init<std::chrono::milliseconds, hws::sample_category>(), "construct a new Intel GPU hardware sampler for the default device with the specified sampling interval sampling only the provided sample_category samples")
        .def(py::init<std::size_t, std::chrono::milliseconds>(), "construct a new Intel GPU hardware sampler for the specified device and sampling interval")
        .def(py::init<std::size_t, std::chrono::milliseconds, hws::sample_category>(), "construct a new Intel GPU hardware sampler for the specified device and sampling interval sampling only the provided sample_category samples")
        .def("general_samples", &hws::gpu_intel_hardware_sampler::general_samples, "get all general samples")
        .def("clock_samples", &hws::gpu_intel_hardware_sampler::clock_samples, "get all clock related samples")
        .def("power_samples", &hws::gpu_intel_hardware_sampler::power_samples, "get all power related samples")
        .def("memory_samples", &hws::gpu_intel_hardware_sampler::memory_samples, "get all memory related samples")
        .def("temperature_samples", &hws::gpu_intel_hardware_sampler::temperature_samples, "get all temperature related samples")
        .def("__repr__", [](const hws::gpu_intel_hardware_sampler &self) {
            return fmt::format("<HardwareSampling.GpuIntelHardwareSampler with\n{}\n>", self);
        });
}
