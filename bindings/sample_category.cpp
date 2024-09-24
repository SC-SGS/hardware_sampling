/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hardware_sampling/sample_category.hpp"  // hws::sample_category

#include "pybind11/operators.h"  // operator overloading
#include "pybind11/pybind11.h"   // py::module_, py::overload_cast

namespace py = pybind11;

void init_sample_category(py::module_ &m) {
    // sample_category enum and bitwise operations on the sample_category enum
    py::enum_<hws::sample_category>(m, "SampleCategory")
        .value("GENERAL", hws::sample_category::general, "General hardware samples like architecture, names, or utilization.")
        .value("CLOCK", hws::sample_category::clock, "Clock-related hardware samples like minimum, maximum, and current frequencies or throttle reasons.")
        .value("POWER", hws::sample_category::power, "Power-related hardware samples like current power draw or total energy consumption.")
        .value("MEMORY", hws::sample_category::memory, "Memory-related hardware samples like memory usage or PCIe information.")
        .value("TEMPERATURE", hws::sample_category::temperature, "Temperature-related hardware samples like maximum and current temperatures.")
        .value("GFX", hws::sample_category::gfx, "Gfx-related (iGPU) hardware samples. Only used in the cpu_hardware_sampler.")
        .value("IDLE_STATE", hws::sample_category::idle_state, "Idle-state-related hardware samples. Only used in the cpu_hardware_sampler.")
        .value("ALL", hws::sample_category::all, "Shortcut to enable all available hardware samples (default).")
        .def("__invert__", py::overload_cast<hws::sample_category>(&hws::operator~))
        .def("__and__", py::overload_cast<hws::sample_category, hws::sample_category>(&hws::operator&))
        .def("__or__", py::overload_cast<hws::sample_category, hws::sample_category>(&hws::operator|))
        .def("__xor__", py::overload_cast<hws::sample_category, hws::sample_category>(&hws::operator^));
}
