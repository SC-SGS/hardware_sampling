/**
* @author Marcel Breyer
* @copyright 2024-today All Rights Reserved
* @license This file is released under the MIT license.
*          See the LICENSE.md file in the project root for full license information.
*/

#include "hardware_sampling/version.hpp"  // hws::version

#include "pybind11/pybind11.h"   // py::module_

namespace py = pybind11;

// dummy class
class version { };

void init_version(py::module_ &m) {
    // bind global version information
    // complexity necessary to enforce read-only
    py::class_<version>(m, "version")
        .def_property_readonly_static("name", [](const py::object & /* self */) { return hws::version::name; }, "the name of the hws library")
        .def_property_readonly_static("version", [](const py::object & /* self */) { return hws::version::version; }, "the used version of the hws library")
        .def_property_readonly_static("major", [](const py::object & /* self */) { return hws::version::major; }, "the used major version of the hws library")
        .def_property_readonly_static("minor", [](const py::object & /* self */) { return hws::version::minor; }, "the used minor version of the hws library")
        .def_property_readonly_static("patch", [](const py::object & /* self */) { return hws::version::patch; }, "the used patch version of the hws library");
}
