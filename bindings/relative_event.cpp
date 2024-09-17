/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "relative_event.hpp"  // hws::detail::relative_event

#include "fmt/format.h"         // fmt::format
#include "pybind11/chrono.h"    // bind std::chrono types
#include "pybind11/pybind11.h"  // py::module_
#include "pybind11/stl.h"       // bind STL types

namespace py = pybind11;

void init_relative_event(py::module_ &m) {
    // a special python only struct encapsulating a relative event, i.e., an event where its "relative_time_point" member is the time passed since the first event
    py::class_<hws::detail::relative_event>(m, "RelativeEvent")
        .def(py::init<decltype(hws::detail::relative_event::relative_time_point), decltype(hws::detail::relative_event::name)>(), "construct a new event using a time point and a name")
        .def_readonly("relative_time_point", &hws::detail::relative_event::relative_time_point, "read the relative time point associated to this event")
        .def_readonly("name", &hws::detail::relative_event::name, "read the name associated to this event")
        .def("__repr__", [](const hws::detail::relative_event &self) {
            return fmt::format("<HardWareSampling.RelativeEvent with {{ time_point: {}, name: {} }}>", self.relative_time_point, self.name);
        });
}
