/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hardware_sampling/event.hpp"  // hws::event

#include "pybind11/pybind11.h"  // py::module_
#include "pybind11/stl.h"       // bind STL types

#include <format>  // std::format

namespace py = pybind11;

void init_event(py::module_ &m) {
    const py::module_ hardware_sampling_module = m.def_submodule("HardwareSampling");

    // bind a single event
    py::class_<hws::event>(hardware_sampling_module, "Event")
        .def(py::init<decltype(hws::event::time_point), decltype(hws::event::name)>(), "construct a new event using a time point and a name")
        .def_readonly("time_point", &hws::event::time_point, "read the time point associated to this event")
        .def_readonly("name", &hws::event::name, "read the name associated to this event")
        .def("__repr__", [](const hws::event &self) {
            return std::format("<HardWareSampling.Event with {{ time_point: {}, name: {} }}>", self.time_point.time_since_epoch(), self.name);
        });
}