/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hardware_sampling/gpu_amd/hardware_sampler.hpp"  // hws::gpu_amd_hardware_sampler
#include "hardware_sampling/gpu_amd/rocm_smi_samples.hpp"  // hws::{rocm_smi_general_samples, rocm_smi_clock_samples, rocm_smi_power_samples, rocm_smi_memory_samples, rocm_smi_temperature_samples}
#include "hardware_sampling/hardware_sampler.hpp"          // hws::hardware_sampler
#include "hardware_sampling/sample_category.hpp"           // hws::sample_category

#include "fmt/format.h"         // fmt::format
#include "pybind11/chrono.h"    // automatic bindings for std::chrono::milliseconds
#include "pybind11/pybind11.h"  // py::module_
#include "pybind11/stl.h"       // bind STL types

#include <chrono>   // std::chrono::milliseconds
#include <cstddef>  // std::size_t

namespace py = pybind11;

void init_gpu_amd_hardware_sampler(py::module_ &m) {
    // bind the general samples
    py::class_<hws::rocm_smi_general_samples>(m, "RocmSmiGeneralSamples")
        .def("has_samples", &hws::rocm_smi_general_samples::has_samples, "true if any sample is available, false otherwise")
        .def("get_architecture", &hws::rocm_smi_general_samples::get_name, "the architecture name of the device")
        .def("get_byte_order", &hws::rocm_smi_general_samples::get_byte_order, "the byte order (e.g., little/big endian)")
        .def("get_vendor_id", &hws::rocm_smi_general_samples::get_vendor_id, "the vendor ID")
        .def("get_name", &hws::rocm_smi_general_samples::get_name, "the name of the device")
        .def("get_compute_utilization", &hws::rocm_smi_general_samples::get_compute_utilization, "the GPU compute utilization in percent")
        .def("get_memory_utilization", &hws::rocm_smi_general_samples::get_memory_utilization, "the GPU memory utilization in percent")
        .def("get_performance_level", &hws::rocm_smi_general_samples::get_performance_level, "the performance level: one of rsmi_dev_perf_level_t")
        .def("__repr__", [](const hws::rocm_smi_general_samples &self) {
            return fmt::format("<HardwareSampling.RocmSmiGeneralSamples with\n{}\n>", self);
        });

    // bind the clock samples
    py::class_<hws::rocm_smi_clock_samples>(m, "RocmSmiClockSamples")
        .def("has_samples", &hws::rocm_smi_clock_samples::has_samples, "true if any sample is available, false otherwise")
        .def("get_clock_frequency_min", &hws::rocm_smi_clock_samples::get_clock_frequency_min, "the minimum possible system clock frequency in MHz")
        .def("get_clock_frequency_max", &hws::rocm_smi_clock_samples::get_clock_frequency_max, "the maximum possible system clock frequency in MHz")
        .def("get_memory_clock_frequency_min", &hws::rocm_smi_clock_samples::get_memory_clock_frequency_min, "the minimum possible memory clock frequency in MHz")
        .def("get_memory_clock_frequency_max", &hws::rocm_smi_clock_samples::get_memory_clock_frequency_max, "the maximum possible memory clock frequency in MHz")
        .def("get_socket_clock_frequency_min", &hws::rocm_smi_clock_samples::get_socket_clock_frequency_min, "the minimum possible socket clock frequency in MHz")
        .def("get_socket_clock_frequency_max", &hws::rocm_smi_clock_samples::get_socket_clock_frequency_max, "the maximum possible socket clock frequency in MHz")
        .def("get_available_clock_frequencies", &hws::rocm_smi_clock_samples::get_available_clock_frequencies, "the available clock frequencies in MHz (slowest to fastest)")
        .def("get_available_memory_clock_frequencies", &hws::rocm_smi_clock_samples::get_available_memory_clock_frequencies, "the available memory clock frequencies in MHz (slowest to fastest)")
        .def("get_clock_frequency", &hws::rocm_smi_clock_samples::get_clock_frequency, "the current system clock frequency in MHz")
        .def("get_memory_clock_frequency", &hws::rocm_smi_clock_samples::get_memory_clock_frequency, "the current memory clock frequency in MHz")
        .def("get_socket_clock_frequency", &hws::rocm_smi_clock_samples::get_socket_clock_frequency, "the current socket clock frequency in MHz")
        .def("get_overdrive_level", &hws::rocm_smi_clock_samples::get_overdrive_level, "the GPU overdrive percentage")
        .def("get_memory_overdrive_level", &hws::rocm_smi_clock_samples::get_memory_overdrive_level, "the GPU's memory overdrive percentage")
        .def("__repr__", [](const hws::rocm_smi_clock_samples &self) {
            return fmt::format("<HardwareSampling.RocmSmiClockSamples with\n{}\n>", self);
        });

    // bind the power samples
    py::class_<hws::rocm_smi_power_samples>(m, "RocmSmiPowerSamples")
        .def("has_samples", &hws::rocm_smi_power_samples::has_samples, "true if any sample is available, false otherwise")
        .def("get_power_management_limit", &hws::rocm_smi_power_samples::get_power_management_limit, "the default power cap (W), may be different from power cap")
        .def("get_power_enforced_limit", &hws::rocm_smi_power_samples::get_power_enforced_limit, "if the GPU draws more power (W) than the power cap, the GPU may throttle")
        .def("get_power_measurement_type", &hws::rocm_smi_power_samples::get_power_measurement_type, "the type of the power readings: either current power draw or average power draw")
        .def("get_available_power_profiles", &hws::rocm_smi_power_samples::get_available_power_profiles, "a list of the available power profiles")
        .def("get_power_usage", &hws::rocm_smi_power_samples::get_power_usage, "the current GPU socket power draw in W")
        .def("get_power_total_energy_consumption", &hws::rocm_smi_power_samples::get_power_total_energy_consumption, "the total power consumption since the last driver reload in J")
        .def("get_power_profile", &hws::rocm_smi_power_samples::get_power_profile, "the current active power profile; one of 'available_power_profiles'")
        .def("__repr__", [](const hws::rocm_smi_power_samples &self) {
            return fmt::format("<HardwareSampling.RocmSmiPowerSamples with\n{}\n>", self);
        });

    // bind the memory samples
    py::class_<hws::rocm_smi_memory_samples>(m, "RocmSmiMemorySamples")
        .def("has_samples", &hws::rocm_smi_memory_samples::has_samples, "true if any sample is available, false otherwise")
        .def("get_memory_total", &hws::rocm_smi_memory_samples::get_memory_total, "the total available memory in Byte")
        .def("get_visible_memory_total", &hws::rocm_smi_memory_samples::get_visible_memory_total, "the total visible available memory in Byte, may be smaller than the total memory")
        .def("get_num_pcie_lanes_min", &hws::rocm_smi_memory_samples::get_num_pcie_lanes_min, "the minimum number of used PCIe lanes")
        .def("get_num_pcie_lanes_max", &hws::rocm_smi_memory_samples::get_num_pcie_lanes_max, "the maximum number of used PCIe lanes")
        .def("get_pcie_link_transfer_rate_min", &hws::rocm_smi_memory_samples::get_pcie_link_transfer_rate_min, "the minimum PCIe link transfer rate in MT/s")
        .def("get_pcie_link_transfer_rate_max", &hws::rocm_smi_memory_samples::get_pcie_link_transfer_rate_max, "the maximum PCIe link transfer rate in MT/s")
        .def("get_memory_used", &hws::rocm_smi_memory_samples::get_memory_used, "the currently used memory in Byte")
        .def("get_memory_free", &hws::rocm_smi_memory_samples::get_memory_free, "the currently free memory in Byte")
        .def("get_num_pcie_lanes", &hws::rocm_smi_memory_samples::get_num_pcie_lanes, "the number of currently used PCIe lanes")
        .def("get_pcie_link_transfer_rate", &hws::rocm_smi_memory_samples::get_pcie_link_transfer_rate, "the current PCIe transfer rate in T/s")
        .def("__repr__", [](const hws::rocm_smi_memory_samples &self) {
            return fmt::format("<HardwareSampling.RocmSmiMemorySamples with\n{}\n>", self);
        });

    // bind the temperature samples
    py::class_<hws::rocm_smi_temperature_samples>(m, "RocmSmiTemperatureSamples")
        .def("has_samples", &hws::rocm_smi_temperature_samples::has_samples, "true if any sample is available, false otherwise")
        .def("get_num_fans", &hws::rocm_smi_temperature_samples::get_num_fans, "the number of fans (if any)")
        .def("get_fan_speed_max", &hws::rocm_smi_temperature_samples::get_fan_speed_max, "the maximum fan speed in RPM")
        .def("get_temperature_min", &hws::rocm_smi_temperature_samples::get_temperature_min, "the minimum temperature on the GPU's edge temperature sensor in °C")
        .def("get_temperature_max", &hws::rocm_smi_temperature_samples::get_temperature_max, "the maximum temperature on the GPU's edge temperature sensor in °C")
        .def("get_memory_temperature_min", &hws::rocm_smi_temperature_samples::get_memory_temperature_min, "the minimum temperature on the GPU's memory temperature sensor in °C")
        .def("get_memory_temperature_max", &hws::rocm_smi_temperature_samples::get_memory_temperature_max, "the maximum temperature on the GPU's memory temperature sensor in °C")
        .def("get_hotspot_temperature_min", &hws::rocm_smi_temperature_samples::get_hotspot_temperature_min, "the minimum temperature on the GPU's hotspot temperature sensor in °C")
        .def("get_hotspot_temperature_max", &hws::rocm_smi_temperature_samples::get_hotspot_temperature_max, "the maximum temperature on the GPU's hotspot temperature sensor in °C")
        .def("get_hbm_0_temperature_min", &hws::rocm_smi_temperature_samples::get_hbm_0_temperature_min, "the minimum temperature on the GPU's HBM0 temperature sensor in °C")
        .def("get_hbm_0_temperature_max", &hws::rocm_smi_temperature_samples::get_hbm_0_temperature_max, "the maximum temperature on the GPU's HBM0 temperature sensor in °C")
        .def("get_hbm_1_temperature_min", &hws::rocm_smi_temperature_samples::get_hbm_1_temperature_min, "the minimum temperature on the GPU's HBM1 temperature sensor in °C")
        .def("get_hbm_1_temperature_max", &hws::rocm_smi_temperature_samples::get_hbm_1_temperature_max, "the maximum temperature on the GPU's HBM1 temperature sensor in °C")
        .def("get_hbm_2_temperature_min", &hws::rocm_smi_temperature_samples::get_hbm_2_temperature_min, "the minimum temperature on the GPU's HBM2 temperature sensor in °C")
        .def("get_hbm_2_temperature_max", &hws::rocm_smi_temperature_samples::get_hbm_2_temperature_max, "the maximum temperature on the GPU's HBM2 temperature sensor in °C")
        .def("get_hbm_3_temperature_min", &hws::rocm_smi_temperature_samples::get_hbm_3_temperature_min, "the minimum temperature on the GPU's HBM3 temperature sensor in °C")
        .def("get_hbm_3_temperature_max", &hws::rocm_smi_temperature_samples::get_hbm_3_temperature_max, "the maximum temperature on the GPU's HBM3 temperature sensor in °C")
        .def("get_fan_speed_percentage", &hws::rocm_smi_temperature_samples::get_fan_speed_percentage, "the current fan speed in %")
        .def("get_temperature", &hws::rocm_smi_temperature_samples::get_temperature, "the current temperature on the GPU's edge temperature sensor in °C")
        .def("get_hotspot_temperature", &hws::rocm_smi_temperature_samples::get_hotspot_temperature, "the current temperature on the GPU's hotspot temperature sensor in °C")
        .def("get_memory_temperature", &hws::rocm_smi_temperature_samples::get_memory_temperature, "the current temperature on the GPU's memory temperature sensor in °C")
        .def("get_hbm_0_temperature", &hws::rocm_smi_temperature_samples::get_hbm_0_temperature, "the current temperature on the GPU's HBM0 temperature sensor in °C")
        .def("get_hbm_1_temperature", &hws::rocm_smi_temperature_samples::get_hbm_1_temperature, "the current temperature on the GPU's HBM1 temperature sensor in °C")
        .def("get_hbm_2_temperature", &hws::rocm_smi_temperature_samples::get_hbm_2_temperature, "the current temperature on the GPU's HBM2 temperature sensor in °C")
        .def("get_hbm_3_temperature", &hws::rocm_smi_temperature_samples::get_hbm_3_temperature, "the current temperature on the GPU's HBM3 temperature sensor in °C")
        .def("__repr__", [](const hws::rocm_smi_temperature_samples &self) {
            return fmt::format("<HardwareSampling.RocmSmiTemperatureSamples with\n{}\n>", self);
        });

    // bind the GPU AMD hardware sampler class
    py::class_<hws::gpu_amd_hardware_sampler, hws::hardware_sampler>(m, "GpuAmdHardwareSampler")
        .def(py::init<>(), "construct a new AMD GPU hardware sampler for the default device with the default sampling interval")
        .def(py::init<hws::sample_category>(), "construct a new AMD GPU hardware sampler for the default device with the default sampling interval sampling only the provided sample_category samples")
        .def(py::init<std::size_t>(), "construct a new AMD GPU hardware sampler for the specified device with the default sampling interval")
        .def(py::init<std::size_t, hws::sample_category>(), "construct a new AMD GPU hardware sampler for the specified device with the default sampling interval sampling only the provided sample_category samples")
        .def(py::init<std::chrono::milliseconds>(), "construct a new AMD GPU hardware sampler for the default device with the specified sampling interval")
        .def(py::init<std::chrono::milliseconds, hws::sample_category>(), "construct a new AMD GPU hardware sampler for the default device with the specified sampling interval sampling only the provided sample_category samples")
        .def(py::init<std::size_t, std::chrono::milliseconds>(), "construct a new AMD GPU hardware sampler for the specified device and sampling interval")
        .def(py::init<std::size_t, std::chrono::milliseconds, hws::sample_category>(), "construct a new AMD GPU hardware sampler for the specified device and sampling interval sampling only the provided sample_category samples")
        .def("general_samples", &hws::gpu_amd_hardware_sampler::general_samples, "get all general samples")
        .def("clock_samples", &hws::gpu_amd_hardware_sampler::clock_samples, "get all clock related samples")
        .def("power_samples", &hws::gpu_amd_hardware_sampler::power_samples, "get all power related samples")
        .def("memory_samples", &hws::gpu_amd_hardware_sampler::memory_samples, "get all memory related samples")
        .def("temperature_samples", &hws::gpu_amd_hardware_sampler::temperature_samples, "get all temperature related samples")
        .def("__repr__", [](const hws::gpu_amd_hardware_sampler &self) {
            return fmt::format("<HardwareSampling.GpuAmdHardwareSampler with\n{}\n>", self);
        });
}
