/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hardware_sampling/gpu_amd/hardware_sampler.hpp"  // hws::gpu_amd_hardware_sampler
#include "hardware_sampling/gpu_amd/rocm_smi_samples.hpp"  // hws::{rocm_smi_general_samples, rocm_smi_clock_samples, rocm_smi_power_samples, rocm_smi_memory_samples, rocm_smi_temperature_samples}
#include "hardware_sampling/hardware_sampler.hpp"          // hws::hardware_sampler

#include "pybind11/chrono.h"    // automatic bindings for std::chrono::milliseconds
#include "pybind11/pybind11.h"  // py::module_
#include "pybind11/stl.h"       // bind STL types

#include <chrono>   // std::chrono::milliseconds
#include <cstddef>  // std::size_t
#include <format>   // std::format

namespace py = pybind11;

void init_gpu_amd_hardware_sampler(py::module_ &m) {
    // bind the general samples
    py::class_<hws::rocm_smi_general_samples>(m, "RocmSmiGeneralSamples")
        .def("get_name", &hws::rocm_smi_general_samples::get_name, "the name of the device")
        .def("get_performance_level", &hws::rocm_smi_general_samples::get_performance_level, "the performance level: one of rsmi_dev_perf_level_t")
        .def("get_utilization_gpu", &hws::rocm_smi_general_samples::get_utilization_gpu, "the GPU compute utilization in percent")
        .def("get_utilization_mem", &hws::rocm_smi_general_samples::get_utilization_mem, "the GPU memory utilization in percent")
        .def("__repr__", [](const hws::rocm_smi_general_samples &self) {
            return std::format("<HardwareSampling.RocmSmiGeneralSamples with\n{}\n>", self);
        });

    // bind the clock samples
    py::class_<hws::rocm_smi_clock_samples>(m, "RocmSmiClockSamples")
        .def("get_clock_system_min", &hws::rocm_smi_clock_samples::get_clock_system_min, "the minimum possible system clock frequency in Hz")
        .def("get_clock_system_max", &hws::rocm_smi_clock_samples::get_clock_system_max, "the maximum possible system clock frequency in Hz")
        .def("get_clock_socket_min", &hws::rocm_smi_clock_samples::get_clock_socket_min, "the minimum possible socket clock frequency in Hz")
        .def("get_clock_socket_max", &hws::rocm_smi_clock_samples::get_clock_socket_max, "the maximum possible socket clock frequency in Hz")
        .def("get_clock_memory_min", &hws::rocm_smi_clock_samples::get_clock_memory_min, "the minimum possible memory clock frequency in Hz")
        .def("get_clock_memory_max", &hws::rocm_smi_clock_samples::get_clock_memory_max, "the maximum possible memory clock frequency in Hz")
        .def("get_clock_system", &hws::rocm_smi_clock_samples::get_clock_system, "the current system clock frequency in Hz")
        .def("get_clock_socket", &hws::rocm_smi_clock_samples::get_clock_socket, "the current socket clock frequency in Hz")
        .def("get_clock_memory", &hws::rocm_smi_clock_samples::get_clock_memory, "the current memory clock frequency in Hz")
        .def("get_overdrive_level", &hws::rocm_smi_clock_samples::get_overdrive_level, "the GPU overdrive percentage")
        .def("get_memory_overdrive_level", &hws::rocm_smi_clock_samples::get_memory_overdrive_level, "the GPU's memory overdrive percentage")
        .def("__repr__", [](const hws::rocm_smi_clock_samples &self) {
            return std::format("<HardwareSampling.RocmSmiClockSamples with\n{}\n>", self);
        });

    // bind the power samples
    py::class_<hws::rocm_smi_power_samples>(m, "RocmSmiPowerSamples")
        .def("get_power_default_cap", &hws::rocm_smi_power_samples::get_power_default_cap, "the default power cap, may be different from power cap")
        .def("get_power_cap", &hws::rocm_smi_power_samples::get_power_cap, "if the GPU draws more power (μW) than the power cap, the GPU may throttle")
        .def("get_power_type", &hws::rocm_smi_power_samples::get_power_type, "the type of the power management: either current power draw or average power draw")
        .def("get_available_power_profiles", &hws::rocm_smi_power_samples::get_available_power_profiles, "a list of the available power profiles")
        .def("get_power_usage", &hws::rocm_smi_power_samples::get_power_usage, "the current GPU socket power draw in μW")
        .def("get_power_total_energy_consumption", &hws::rocm_smi_power_samples::get_power_total_energy_consumption, "the total power consumption since the last driver reload in μJ")
        .def("get_power_profile", &hws::rocm_smi_power_samples::get_power_profile, "the current active power profile; one of 'available_power_profiles'")
        .def("__repr__", [](const hws::rocm_smi_power_samples &self) {
            return std::format("<HardwareSampling.RocmSmiPowerSamples with\n{}\n>", self);
        });

    // bind the memory samples
    py::class_<hws::rocm_smi_memory_samples>(m, "RocmSmiMemorySamples")
        .def("get_memory_total", &hws::rocm_smi_memory_samples::get_memory_total, "the total available memory in Byte")
        .def("get_visible_memory_total", &hws::rocm_smi_memory_samples::get_visible_memory_total, "the total visible available memory in Byte, may be smaller than the total memory")
        .def("get_min_num_pcie_lanes", &hws::rocm_smi_memory_samples::get_min_num_pcie_lanes, "the minimum number of used PCIe lanes")
        .def("get_max_num_pcie_lanes", &hws::rocm_smi_memory_samples::get_max_num_pcie_lanes, "the maximum number of used PCIe lanes")
        .def("get_memory_used", &hws::rocm_smi_memory_samples::get_memory_used, "the currently used memory in Byte")
        .def("get_pcie_transfer_rate", &hws::rocm_smi_memory_samples::get_pcie_transfer_rate, "the current PCIe transfer rate in T/s")
        .def("get_num_pcie_lanes", &hws::rocm_smi_memory_samples::get_num_pcie_lanes, "the number of currently used PCIe lanes")
        .def("__repr__", [](const hws::rocm_smi_memory_samples &self) {
            return std::format("<HardwareSampling.RocmSmiMemorySamples with\n{}\n>", self);
        });

    // bind the temperature samples
    py::class_<hws::rocm_smi_temperature_samples>(m, "RocmSmiTemperatureSamples")
        .def("get_num_fans", &hws::rocm_smi_temperature_samples::get_num_fans, "the number of fans (if any)")
        .def("get_max_fan_speed", &hws::rocm_smi_temperature_samples::get_max_fan_speed, "the maximum fan speed")
        .def("get_temperature_edge_min", &hws::rocm_smi_temperature_samples::get_temperature_edge_min, "the minimum temperature on the GPU's edge temperature sensor in m°C")
        .def("get_temperature_edge_max", &hws::rocm_smi_temperature_samples::get_temperature_edge_max, "the maximum temperature on the GPU's edge temperature sensor in m°C")
        .def("get_temperature_hotspot_min", &hws::rocm_smi_temperature_samples::get_temperature_hotspot_min, "the minimum temperature on the GPU's hotspot temperature sensor in m°C")
        .def("get_temperature_hotspot_max", &hws::rocm_smi_temperature_samples::get_temperature_hotspot_max, "the maximum temperature on the GPU's hotspot temperature sensor in m°C")
        .def("get_temperature_memory_min", &hws::rocm_smi_temperature_samples::get_temperature_memory_min, "the minimum temperature on the GPU's memory temperature sensor in m°C")
        .def("get_temperature_memory_max", &hws::rocm_smi_temperature_samples::get_temperature_memory_max, "the maximum temperature on the GPU's memory temperature sensor in m°C")
        .def("get_temperature_hbm_0_min", &hws::rocm_smi_temperature_samples::get_temperature_hbm_0_min, "the minimum temperature on the GPU's HBM0 temperature sensor in m°C")
        .def("get_temperature_hbm_0_max", &hws::rocm_smi_temperature_samples::get_temperature_hbm_0_max, "the maximum temperature on the GPU's HBM0 temperature sensor in m°C")
        .def("get_temperature_hbm_1_min", &hws::rocm_smi_temperature_samples::get_temperature_hbm_1_min, "the minimum temperature on the GPU's HBM1 temperature sensor in m°C")
        .def("get_temperature_hbm_1_max", &hws::rocm_smi_temperature_samples::get_temperature_hbm_1_max, "the maximum temperature on the GPU's HBM1 temperature sensor in m°C")
        .def("get_temperature_hbm_2_min", &hws::rocm_smi_temperature_samples::get_temperature_hbm_2_min, "the minimum temperature on the GPU's HBM2 temperature sensor in m°C")
        .def("get_temperature_hbm_2_max", &hws::rocm_smi_temperature_samples::get_temperature_hbm_2_max, "the maximum temperature on the GPU's HBM2 temperature sensor in m°C")
        .def("get_temperature_hbm_3_min", &hws::rocm_smi_temperature_samples::get_temperature_hbm_3_min, "the minimum temperature on the GPU's HBM3 temperature sensor in m°C")
        .def("get_temperature_hbm_3_max", &hws::rocm_smi_temperature_samples::get_temperature_hbm_3_max, "the maximum temperature on the GPU's HBM3 temperature sensor in m°C")
        .def("get_fan_speed", &hws::rocm_smi_temperature_samples::get_fan_speed, "the current fan speed in %")
        .def("get_temperature_edge", &hws::rocm_smi_temperature_samples::get_temperature_edge, "the current temperature on the GPU's edge temperature sensor in m°C")
        .def("get_temperature_hotspot", &hws::rocm_smi_temperature_samples::get_temperature_hotspot, "the current temperature on the GPU's hotspot temperature sensor in m°C")
        .def("get_temperature_memory", &hws::rocm_smi_temperature_samples::get_temperature_memory, "the current temperature on the GPU's memory temperature sensor in m°C")
        .def("get_temperature_hbm_0", &hws::rocm_smi_temperature_samples::get_temperature_hbm_0, "the current temperature on the GPU's HBM0 temperature sensor in m°C")
        .def("get_temperature_hbm_1", &hws::rocm_smi_temperature_samples::get_temperature_hbm_1, "the current temperature on the GPU's HBM1 temperature sensor in m°C")
        .def("get_temperature_hbm_2", &hws::rocm_smi_temperature_samples::get_temperature_hbm_2, "the current temperature on the GPU's HBM2 temperature sensor in m°C")
        .def("get_temperature_hbm_3", &hws::rocm_smi_temperature_samples::get_temperature_hbm_3, "the current temperature on the GPU's HBM3 temperature sensor in m°C")
        .def("__repr__", [](const hws::rocm_smi_temperature_samples &self) {
            return std::format("<HardwareSampling.RocmSmiTemperatureSamples with\n{}\n>", self);
        });

    // bind the GPU AMD hardware sampler class
    py::class_<hws::gpu_amd_hardware_sampler, hws::hardware_sampler>(m, "GpuAmdHardwareSampler")
        .def(py::init<>(), "construct a new AMD GPU hardware sampler for the default device with the default sampling interval")
        .def(py::init<std::size_t>(), "construct a new AMD GPU hardware sampler for the specified device with the default sampling interval")
        .def(py::init<std::chrono::milliseconds>(), "construct a new AMD GPU hardware sampler for the default device with the specified sampling interval")
        .def(py::init<std::size_t, std::chrono::milliseconds>(), "construct a new AMD GPU hardware sampler for the specified device and sampling interval")
        .def("general_samples", &hws::gpu_amd_hardware_sampler::general_samples, "get all general samples")
        .def("clock_samples", &hws::gpu_amd_hardware_sampler::clock_samples, "get all clock related samples")
        .def("power_samples", &hws::gpu_amd_hardware_sampler::power_samples, "get all power related samples")
        .def("memory_samples", &hws::gpu_amd_hardware_sampler::memory_samples, "get all memory related samples")
        .def("temperature_samples", &hws::gpu_amd_hardware_sampler::temperature_samples, "get all temperature related samples")
        .def("__repr__", [](const hws::gpu_amd_hardware_sampler &self) {
            return std::format("<HardwareSampling.GpuAmdHardwareSampler with\n{}\n>", self);
        });
}
