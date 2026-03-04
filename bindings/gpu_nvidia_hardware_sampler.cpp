/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hws/gpu_nvidia/hardware_sampler.hpp"  // hws::gpu_nvidia_hardware_sampler
#include "hws/gpu_nvidia/nvml_samples.hpp"      // hws::{nvml_general_samples, nvml_clock_samples, nvml_power_samples, nvml_memory_samples, nvml_temperature_samples}
#include "hws/hardware_sampler.hpp"             // hws::hardware_sampler
#include "hws/sample_category.hpp"              // hws::sample_category

#include "fmt/format.h"         // fmt::format
#include "pybind11/chrono.h"    // automatic bindings for std::chrono::milliseconds
#include "pybind11/pybind11.h"  // py::module_
#include "pybind11/stl.h"       // bind STL types

#include <chrono>   // std::chrono::milliseconds
#include <cstddef>  // std::size_t

namespace py = pybind11;

void init_gpu_nvidia_hardware_sampler(py::module_ &m) {
    // bind the general samples
    py::class_<hws::nvml_general_samples>(m, "NvmlGeneralSamples")
        .def("has_samples", &hws::nvml_general_samples::has_samples, "true if any sample is available, false otherwise")
        .def("get_architecture", &hws::nvml_general_samples::get_architecture, "the architecture name of the device")
        .def("get_byte_order", &hws::nvml_general_samples::get_byte_order, "the byte order (e.g., little/big endian)")
        .def("get_num_cores", &hws::nvml_general_samples::get_num_cores, "the number of CUDA cores")
        .def("get_vendor_id", &hws::nvml_general_samples::get_vendor_id, "the vendor ID")
        .def("get_name", &hws::nvml_general_samples::get_name, "the name of the device")
        .def("get_persistence_mode", &hws::nvml_general_samples::get_persistence_mode, "the persistence mode: if true, the driver is always loaded reducing the latency for the first API call")
        .def("get_compute_utilization", &hws::nvml_general_samples::get_compute_utilization, "the GPU compute utilization in percent")
        .def("get_memory_utilization", &hws::nvml_general_samples::get_memory_utilization, "the GPU memory utilization in percent")
        .def("get_performance_level", &hws::nvml_general_samples::get_performance_level, "the performance state: 0 - 15 where 0 is the maximum performance and 15 the minimum performance")
        .def("__repr__", [](const hws::nvml_general_samples &self) {
            return fmt::format("<HardwareSampling.NvmlGeneralSamples with\n{}\n>", self);
        });

    // bind the clock samples
    py::class_<hws::nvml_clock_samples>(m, "NvmlClockSamples")
        .def("has_samples", &hws::nvml_clock_samples::has_samples, "true if any sample is available, false otherwise")
        .def("get_auto_boosted_clock_enabled", &hws::nvml_clock_samples::get_auto_boosted_clock_enabled, "true if clock boosting is currently enabled")
        .def("get_clock_frequency_min", &hws::nvml_clock_samples::get_clock_frequency_min, "the minimum possible graphics clock frequency in MHz")
        .def("get_clock_frequency_max", &hws::nvml_clock_samples::get_clock_frequency_max, "the maximum possible graphics clock frequency in MHz")
        .def("get_memory_clock_frequency_min", &hws::nvml_clock_samples::get_memory_clock_frequency_min, "the minimum possible memory clock frequency in MHz")
        .def("get_memory_clock_frequency_max", &hws::nvml_clock_samples::get_memory_clock_frequency_max, "the maximum possible memory clock frequency in MHz")
        .def("get_sm_clock_frequency_max", &hws::nvml_clock_samples::get_sm_clock_frequency_max, "the maximum possible SM clock frequency in MHz")
        .def("get_clock_frequency", &hws::nvml_clock_samples::get_clock_frequency, "the current graphics clock frequency in MHz")
        .def("get_available_clock_frequencies", &hws::nvml_clock_samples::get_available_clock_frequencies, "the available clock frequencies in MHz, based on a memory clock frequency (slowest to fastest)")
        .def("get_available_memory_clock_frequencies", &hws::nvml_clock_samples::get_available_memory_clock_frequencies, "the available memory clock frequencies in MHz (slowest to fastest)")
        .def("get_memory_clock_frequency", &hws::nvml_clock_samples::get_memory_clock_frequency, "the current memory clock frequency in MHz")
        .def("get_sm_clock_frequency", &hws::nvml_clock_samples::get_sm_clock_frequency, "the current SM clock frequency in Mhz")
        .def("get_throttle_reason", &hws::nvml_clock_samples::get_throttle_reason, "the reason the GPU clock throttled (as bitmask)")
        .def("get_throttle_reason_string", &hws::nvml_clock_samples::get_throttle_reason_string, "the reason the GPU clock throttled (as string)")
        .def("get_auto_boosted_clock", &hws::nvml_clock_samples::get_auto_boosted_clock, "true if the clocks are currently auto boosted")
        .def("__repr__", [](const hws::nvml_clock_samples &self) {
            return fmt::format("<HardwareSampling.NvmlClockSamples with\n{}\n>", self);
        });

    // bind the power samples
    py::class_<hws::nvml_power_samples>(m, "NvmlPowerSamples")
        .def("has_samples", &hws::nvml_power_samples::has_samples, "true if any sample is available, false otherwise")
        .def("get_power_management_limit", &hws::nvml_power_samples::get_power_management_limit, "if the GPU draws more power (mW) than the power management limit, the GPU may throttle")
        .def("get_power_enforced_limit", &hws::nvml_power_samples::get_power_enforced_limit, "the actually enforced power limit, may be different from power management limit if external limiters are set")
        .def("get_power_measurement_type", &hws::nvml_power_samples::get_power_measurement_type, "the type of the power readings: either current power draw or average power draw")
        .def("get_power_management_mode", &hws::nvml_power_samples::get_power_management_mode, "true if power management algorithms are supported and active")
        .def("get_available_power_profiles", &hws::nvml_power_samples::get_available_power_profiles, "a list of the available power profiles")
        .def("get_power_usage", &hws::nvml_power_samples::get_power_usage, "the current power draw of the GPU and its related circuity (e.g., memory) in mW")
        .def("get_system_power_usage", &hws::nvml_power_samples::get_system_power_usage, "the current power draw of the GPU+CPU module and its related circuity (e.g., memory) in mW")
        .def("get_power_total_energy_consumption", &hws::nvml_power_samples::get_power_total_energy_consumption, "the total power consumption since the last driver reload in mJ")
        .def("get_power_profile", &hws::nvml_power_samples::get_power_profile, "the current GPU power state: 0 - 15 where 0 is the maximum power and 15 the minimum power")
        .def("__repr__", [](const hws::nvml_power_samples &self) {
            return fmt::format("<HardwareSampling.NvmlPowerSamples with\n{}\n>", self);
        });

    // bind the memory samples
    py::class_<hws::nvml_memory_samples>(m, "NvmlMemorySamples")
        .def("has_samples", &hws::nvml_memory_samples::has_samples, "true if any sample is available, false otherwise")
        .def("get_memory_total", &hws::nvml_memory_samples::get_memory_total, "the total available memory in Byte")
        .def("get_num_pcie_lanes_max", &hws::nvml_memory_samples::get_num_pcie_lanes_max, "the maximum number of PCIe lanes")
        .def("get_pcie_link_generation_max", &hws::nvml_memory_samples::get_pcie_link_generation_max, "the current PCIe link generation (e.g., PCIe 4.0, PCIe 5.0, etc)")
        .def("get_pcie_link_speed_max", &hws::nvml_memory_samples::get_pcie_link_speed_max, "the maximum PCIe link speed in MBPS")
        .def("get_memory_bus_width", &hws::nvml_memory_samples::get_memory_bus_width, "the memory bus with in Bit")
        .def("get_memory_used", &hws::nvml_memory_samples::get_memory_used, "the currently used memory in Byte")
        .def("get_memory_free", &hws::nvml_memory_samples::get_memory_free, "the currently free memory in Byte")
        .def("get_num_pcie_lanes", &hws::nvml_memory_samples::get_num_pcie_lanes, "the current PCIe link width (e.g., x16, x8, x4, etc)")
        .def("get_pcie_link_generation", &hws::nvml_memory_samples::get_pcie_link_generation, "the current PCIe link generation (may change during runtime to save energy)")
        .def("get_pcie_link_speed", &hws::nvml_memory_samples::get_pcie_link_speed, "the current PCIe link speed in MBPS")
        .def("__repr__", [](const hws::nvml_memory_samples &self) {
            return fmt::format("<HardwareSampling.NvmlMemorySamples with\n{}\n>", self);
        });

    // bind the temperature samples
    py::class_<hws::nvml_temperature_samples>(m, "NvmlTemperatureSamples")
        .def("has_samples", &hws::nvml_temperature_samples::has_samples, "true if any sample is available, false otherwise")
        .def("get_num_fans", &hws::nvml_temperature_samples::get_num_fans, "the number of fans (if any)")
        .def("get_fan_speed_min", &hws::nvml_temperature_samples::get_fan_speed_min, "the minimum fan speed the user can set in %")
        .def("get_fan_speed_max", &hws::nvml_temperature_samples::get_fan_speed_max, "the maximum fan speed the user can set in %")
        .def("get_temperature_max", &hws::nvml_temperature_samples::get_temperature_max, "the maximum graphics temperature threshold in °C")
        .def("get_memory_temperature_max", &hws::nvml_temperature_samples::get_memory_temperature_max, "the maximum memory temperature threshold in °C")
        .def("get_fan_speed_percentage", &hws::nvml_temperature_samples::get_fan_speed_percentage, "the current intended fan speed in %")
        .def("get_temperature", &hws::nvml_temperature_samples::get_temperature, "the current GPU temperature in °C")
        .def("__repr__", [](const hws::nvml_temperature_samples &self) {
            return fmt::format("<HardwareSampling.NvmlTemperatureSamples with\n{}\n>", self);
        });

    // bind the GPU NVIDIA hardware sampler class
    py::class_<hws::gpu_nvidia_hardware_sampler, hws::hardware_sampler>(m, "GpuNvidiaHardwareSampler")
        .def(py::init<>(), "construct a new NVIDIA GPU hardware sampler for the default device with the default sampling interval")
        .def(py::init<hws::sample_category>(), "construct a new NVIDIA GPU hardware sampler for the default device with the default sampling interval sampling only the provided sample_category samples")
        .def(py::init<std::size_t>(), "construct a new NVIDIA GPU hardware sampler for the specified device with the default sampling interval")
        .def(py::init<std::size_t, hws::sample_category>(), "construct a new NVIDIA GPU hardware sampler for the specified device with the default sampling interval sampling only the provided sample_category samples")
        .def(py::init<std::chrono::milliseconds>(), "construct a new NVIDIA GPU hardware sampler for the default device with the specified sampling interval")
        .def(py::init<std::chrono::milliseconds, hws::sample_category>(), "construct a new NVIDIA GPU hardware sampler for the default device with the specified sampling interval sampling only the provided sample_category samples")
        .def(py::init<std::size_t, std::chrono::milliseconds>(), "construct a new NVIDIA GPU hardware sampler for the specified device and sampling interval")
        .def(py::init<std::size_t, std::chrono::milliseconds, hws::sample_category>(), "construct a new NVIDIA GPU hardware sampler for the specified device and sampling interval sampling only the provided sample_category samples")
        .def("general_samples", &hws::gpu_nvidia_hardware_sampler::general_samples, "get all general samples")
        .def("clock_samples", &hws::gpu_nvidia_hardware_sampler::clock_samples, "get all clock related samples")
        .def("power_samples", &hws::gpu_nvidia_hardware_sampler::power_samples, "get all power related samples")
        .def("memory_samples", &hws::gpu_nvidia_hardware_sampler::memory_samples, "get all memory related samples")
        .def("temperature_samples", &hws::gpu_nvidia_hardware_sampler::temperature_samples, "get all temperature related samples")
        .def("samples_only_as_yaml_string", &hws::gpu_nvidia_hardware_sampler::samples_only_as_yaml_string, "return all hardware samples as YAML string")
        .def("__repr__", [](const hws::gpu_nvidia_hardware_sampler &self) {
            return fmt::format("<HardwareSampling.GpuNvidiaHardwareSampler with\n{}\n>", self);
        });
}
