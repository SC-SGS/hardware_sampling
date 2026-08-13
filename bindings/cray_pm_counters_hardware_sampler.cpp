/**
 * @author Alexander Van Craen
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hws/cray_pm_counters/hardware_sampler.hpp"       // hws::cray_pm_counters_hardware_sampler
#include "hws/cray_pm_counters/pm_counters_samples.hpp"     // hws::{cray_pm_counters_general_samples, cray_pm_counters_power_samples}
#include "hws/hardware_sampler.hpp"                         // hws::hardware_sampler
#include "hws/sample_category.hpp"                          // hws::sample_category

#include "fmt/format.h"         // fmt::format
#include "pybind11/chrono.h"    // automatic bindings for std::chrono::milliseconds
#include "pybind11/pybind11.h"  // py::module_
#include "pybind11/stl.h"       // bind STL types

#include <chrono>  // std::chrono::milliseconds

namespace py = pybind11;

void init_cray_pm_counters_hardware_sampler(py::module_ &m) {
    // bind the general samples
    py::class_<hws::cray_pm_counters_general_samples>(m, "CrayPmCountersGeneralSamples")
        .def("has_samples", &hws::cray_pm_counters_general_samples::has_samples, "true if any sample is available, false otherwise")
        .def("get_metadata", &hws::cray_pm_counters_general_samples::get_metadata, "the dynamically discovered, non-energy, non-power pm_counters metadata entries (e.g. version, generation, raw_scan_hz, freshness), keyed by file name")
        .def("__repr__", [](const hws::cray_pm_counters_general_samples &self) {
            return fmt::format("<HardwareSampling.CrayPmCountersGeneralSamples with\n{}\n>", self);
        });

    // bind the power samples
    py::class_<hws::cray_pm_counters_power_samples>(m, "CrayPmCountersPowerSamples")
        .def("has_samples", &hws::cray_pm_counters_power_samples::has_samples, "true if any sample is available, false otherwise")
        .def("get_energy_counters", &hws::cray_pm_counters_power_samples::get_energy_counters, "the dynamically discovered measured energy counters in J (cumulative), keyed by file name, e.g. 'energy' (whole node) or 'accel0_energy' (per accelerator)")
        .def("get_power_counters", &hws::cray_pm_counters_power_samples::get_power_counters, "the dynamically discovered measured power counters in W (instantaneous), keyed by file name, e.g. 'power' (whole node) or 'accel0_power' (per accelerator)")
        .def("get_energy_timestamps_us", &hws::cray_pm_counters_power_samples::get_energy_timestamps_us, "per-sample HSS latch timestamp for the energy counters, in us (PM counters v3+ only)")
        .def("get_power_timestamps_us", &hws::cray_pm_counters_power_samples::get_power_timestamps_us, "per-sample HSS latch timestamp for the power counters, in us (PM counters v3+ only)")
        .def("__repr__", [](const hws::cray_pm_counters_power_samples &self) {
            return fmt::format("<HardwareSampling.CrayPmCountersPowerSamples with\n{}\n>", self);
        });

    // bind the Cray pm_counters hardware sampler class
    py::class_<hws::cray_pm_counters_hardware_sampler, hws::hardware_sampler>(m, "CrayPmCountersHardwareSampler")
        .def(py::init<>(), "construct a new Cray pm_counters hardware sampler with the default sampling interval")
        .def(py::init<hws::sample_category>(), "construct a new Cray pm_counters hardware sampler with the default sampling interval sampling only the provided sample_category samples")
        .def(py::init<std::chrono::milliseconds>(), "construct a new Cray pm_counters hardware sampler with the specified sampling interval")
        .def(py::init<std::chrono::milliseconds, hws::sample_category>(), "construct a new Cray pm_counters hardware sampler with the specified sampling interval sampling only the provided sample_category samples")
        .def("general_samples", &hws::cray_pm_counters_hardware_sampler::general_samples, "get all general samples")
        .def("power_samples", &hws::cray_pm_counters_hardware_sampler::power_samples, "get all power related samples")
        .def("discovered_accel_indices", &hws::cray_pm_counters_hardware_sampler::discovered_accel_indices, "the accelerator indices pm_counters actually exposed on this node, derived from the already-sampled accel<N>_energy counter keys")
        .def("samples_only_as_yaml_string", &hws::cray_pm_counters_hardware_sampler::samples_only_as_yaml_string, "return all hardware samples as YAML string")
        .def("__repr__", [](const hws::cray_pm_counters_hardware_sampler &self) {
            return fmt::format("<HardwareSampling.CrayPmCountersHardwareSampler with\n{}\n>", self);
        });
}
