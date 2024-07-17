/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hardware_sampling/events.hpp"  // hws::events

#include "pybind11/pybind11.h"  // py::module_
#include "pybind11/stl.h"       // bind STL types

#include <format>  // std::format

namespace py = pybind11;

void init_events(py::module_ &m) {
    const py::module_ hardware_sampling_module = m.def_submodule("HardwareSampling");

    using event_type = hws::events::event;

    // bind a single event
    py::class_<event_type>(hardware_sampling_module, "Event")
        .def(py::init<decltype(event_type::time_point), decltype(event_type::name)>(), "construct a new event using a time point and a name")
        .def_readonly("time_point", &event_type::time_point, "read the time point associated to this event")
        .def_readonly("name", &event_type::name, "read the name associated to this event")
        .def("__repr__", [](const event_type &self) {
            return std::format("<HardWareSampling.Events.Event with {{ time_point: {}, name: {} }}>", self.time_point.time_since_epoch(), self.name);
        });

    // bind the events wrapper
    py::class_<hws::events>(hardware_sampling_module, "Events")
        .def(py::init<>(), "construct an empty events wrapper")
        .def("add_event", py::overload_cast<event_type>(&hws::events::add_event), "add a new event")
        .def("add_event", py::overload_cast<decltype(event_type::time_point), decltype(event_type::name)>(&hws::events::add_event), "add a new event using a time point and a name")
        .def("at", &hws::events::operator[], "get the i-th event")
        .def("num_events", &hws::events::num_events, "get the number of events")
        .def("empty", &hws::events::empty, "check whether there are any events")
        .def("get_time_points", &hws::events::get_time_points, "get all stored time points")
        .def("get_names", &hws::events::get_names, "get all stored names")
        .def("__repr__", [](const hws::events &self) {
            return std::format("<HardWareSampling.Events with\n{}\n>", self);
        });
}
