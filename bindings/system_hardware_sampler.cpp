/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hardware_sampling/system_hardware_sampler.hpp"  // hws::system_hardware_sampler

#include "hardware_sampling/event.hpp"            // hws::event
#include "hardware_sampling/sample_category.hpp"  // hws::sample_category
#include "hardware_sampling/utility.hpp"          // hws::detail::durations_from_reference_time

#include "fmt/format.h"         // fmt::format
#include "pybind11/chrono.h"    // bind std::chrono types
#include "pybind11/pybind11.h"  // py::module_, py::class_
#include "pybind11/stl.h"       // bind STL types

#include "relative_event.hpp"  // hws::detail::relative_event
#include <string>              // std::string

namespace py = pybind11;

void init_system_hardware_sampler(py::module_ &m) {
    // bind the pure virtual hardware sampler base class
    py::class_<hws::system_hardware_sampler>(m, "SystemHardwareSampler")
        .def(py::init<>(), "construct a new system hardware sampler with the default sampling interval")
        .def(py::init<hws::sample_category>(), "construct a new system hardware sampler with the default sampling interval sampling only the provided sample_category samples")
        .def(py::init<std::chrono::milliseconds>(), "construct a new system hardware sampler for with the specified sampling interval")
        .def(py::init<std::chrono::milliseconds, hws::sample_category>(), "construct a new system hardware sampler for with the specified sampling interval sampling only the provided sample_category samples")
        .def("start", &hws::system_hardware_sampler::start_sampling, "start hardware sampling for all available hardware samplers")
        .def("stop", &hws::system_hardware_sampler::stop_sampling, "stop hardware sampling for all available hardware samplers")
        .def("pause", &hws::system_hardware_sampler::pause_sampling, "pause hardware sampling for all available hardware samplers")
        .def("resume", &hws::system_hardware_sampler::resume_sampling, "resume hardware sampling for all available hardware samplers")
        .def("has_started", &hws::system_hardware_sampler::has_sampling_started, "check whether hardware sampling has already been started for all hardware samplers")
        .def("is_sampling", &hws::system_hardware_sampler::is_sampling, "check whether the hardware sampling is currently active for all hardware samplers")
        .def("has_stopped", &hws::system_hardware_sampler::has_sampling_stopped, "check whether hardware sampling has already been stopped for all hardware samplers")
        .def("add_event", py::overload_cast<hws::event>(&hws::system_hardware_sampler::add_event), "add a new event to all hardware samplers")
        .def("add_event", py::overload_cast<decltype(hws::event::time_point), decltype(hws::event::name)>(&hws::system_hardware_sampler::add_event), "add a new event using a time point and a name to all hardware samplers")
        .def("add_event", py::overload_cast<decltype(hws::event::name)>(&hws::system_hardware_sampler::add_event), "add a new event using a name, the current time is used as time point to all hardware samplers")
        .def("num_events", &hws::system_hardware_sampler::num_events, "get the number of events separately for each hardware sampler")
        .def("get_events", &hws::system_hardware_sampler::get_events, "get all events separately for each hardware sampler")
        .def("get_relative_events", [](const hws::system_hardware_sampler &self) {
             std::vector<std::vector<hws::detail::relative_event>> relative_events{};
             for (const std::vector<hws::event> &events : self.get_events()) {
                 relative_events.emplace_back();
                 for (const hws::event &e : events) {
                     relative_events.back().emplace_back(hws::detail::duration_from_reference_time(e.time_point, events[0].time_point), e.name);
                 }
             }
             return relative_events; }, "get all relative events separately for each hardware sampler")
        .def("time_points", &hws::system_hardware_sampler::sampling_time_points, "get the time points of the respective hardware samples separately for each hardware sampler")
        .def("relative_time_points", [](const hws::system_hardware_sampler &self) {
            std::vector<std::vector<double>> relative_time_points{};
            for (std::size_t s = 0; s < self.num_samplers(); ++s) {
                relative_time_points.emplace_back(hws::detail::durations_from_reference_time(self.sampling_time_points()[s], self.get_events()[s][0].time_point));
            }
            return relative_time_points; }, "get the relative durations of the respective hardware samples in seconds (as \"normal\" number)")
        .def("sampling_interval", &hws::system_hardware_sampler::sampling_interval, "get the sampling interval separately for each hardware sampler (in ms)")
        .def("num_samplers", &hws::system_hardware_sampler::num_samplers, "get the number of hardware samplers available for the whole system")
        .def("samplers", [](hws::system_hardware_sampler &self) {
            std::vector<hws::hardware_sampler*> out{};
            for (auto &ptr : self.samplers()) {
                out.push_back(ptr.get());
            }
            return out; }, "get the hardware samplers available for the whole system")
        .def("sampler", [](hws::system_hardware_sampler &self, const std::size_t idx) { return self.sampler(idx).get(); }, "get the i-th hardware sampler available for the whole system")
        .def("dump_yaml", py::overload_cast<const std::string &>(&hws::system_hardware_sampler::dump_yaml, py::const_), "dump all hardware samples for all hardware samplers to the given YAML file")
        .def("__repr__", [](const hws::system_hardware_sampler &self) { return fmt::format("<hws.SystemHardwareSampler with {} samples>", self.num_samplers()); });
}
