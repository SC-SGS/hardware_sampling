/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hws/system_hardware_sampler.hpp"  // hws::system_hardware_sampler

#include "hws/event.hpp"            // hws::event
#include "hws/sample_category.hpp"  // hws::sample_category
#include "hws/utility.hpp"          // hws::detail::durations_from_reference_time

#include "fmt/format.h"         // fmt::format
#include "pybind11/chrono.h"    // bind std::chrono types
#include "pybind11/pybind11.h"  // py::module_, py::class_
#include "pybind11/stl.h"       // bind STL types

#include "relative_event.hpp"  // hws::detail::relative_event
#include <string>              // std::string

#if defined(HWS_MPI_SUPPORT_ENABLED)
    #include "mpi4py_communicator.hpp"
    #include <mpi.h>
#endif

namespace py = pybind11;

void init_system_hardware_sampler(py::module_ &m) {
#if defined(HWS_MPI_SUPPORT_ENABLED)
    // bind mpi sampling mode enum
    py::enum_<hws::detail::mpi_sampling_mode>(m, "MPISamplingMode")
        .value("PER_RANK", hws::detail::mpi_sampling_mode::per_rank)
        .value("WHOLE_NODE", hws::detail::mpi_sampling_mode::whole_node)
        .export_values();
#endif
    // bind the pure virtual hardware sampler base class
    py::class_<hws::system_hardware_sampler>(m, "SystemHardwareSampler")
        .def(py::init<>(), "construct a new system hardware sampler with the default sampling interval")
        .def(py::init<hws::sample_category>(), "construct a new system hardware sampler with the default sampling interval sampling only the provided sample_category samples")
        .def(py::init<std::chrono::milliseconds>(), "construct a new system hardware sampler for with the specified sampling interval")
        .def(py::init<std::chrono::milliseconds, hws::sample_category>(), "construct a new system hardware sampler for with the specified sampling interval sampling only the provided sample_category samples")
#if defined(HWS_MPI_SUPPORT_ENABLED)
        // MPI-aware constructors

        // (MPI_Comm, mode, category=all)
        .def(py::init([](py::object py_comm,
                         hws::detail::mpi_sampling_mode mode,
                         hws::sample_category category) {
                 MPI_Comm comm = mpi_comm_from_python(py_comm);
                 return std::make_unique<hws::system_hardware_sampler>(comm, mode, category);
             }),
             py::arg("comm"),
             py::arg("mode"),
             py::arg("category") = hws::sample_category::all,
             "construct a new system hardware sampler with the default sampling interval and MPI support using the given mpi4py communicator and sampling mode")

        // (MPI_Comm, mode, sampling_interval, category=all)
        .def(py::init([](py::object py_comm,
                         hws::detail::mpi_sampling_mode mode,
                         std::chrono::milliseconds sampling_interval,
                         hws::sample_category category) {
                 MPI_Comm comm = mpi_comm_from_python(py_comm);
                 return std::make_unique<hws::system_hardware_sampler>(comm, mode, sampling_interval, category);
             }),
             py::arg("comm"),
             py::arg("mode"),
             py::arg("sampling_interval"),
             py::arg("category") = hws::sample_category::all,
             "construct a new system hardware sampler with the specified sampling interval and MPI support using the given mpi4py communicator and sampling mode")

        // Non-MPI overloads
        .def("start", py::overload_cast<>(&hws::system_hardware_sampler::start_sampling), "start hardware sampling for all available hardware samplers")
        .def("stop", py::overload_cast<>(&hws::system_hardware_sampler::stop_sampling), "stop hardware sampling for all available hardware samplers")
        // MPI-aware overloads
        .def("start", [](hws::system_hardware_sampler &self, py::object py_comm) {
                 MPI_Comm comm = mpi_comm_from_python(py_comm);
                 self.start_sampling(comm); }, py::arg("comm"), "start hardware sampling for all available hardware samplers; executes an MPI barrier on the given communicator before starting")
        .def("stop", [](hws::system_hardware_sampler &self, py::object py_comm) {
                 MPI_Comm comm = mpi_comm_from_python(py_comm);
                 self.stop_sampling(comm); }, py::arg("comm"), "stop hardware sampling for all available hardware samplers; executes an MPI barrier on the given communicator after stopping")
#else
        // No MPI support: only the simple overloads exist, no ambiguity
        .def("start", &hws::system_hardware_sampler::start_sampling, "start hardware sampling for all available hardware samplers")
        .def("stop", &hws::system_hardware_sampler::stop_sampling, "stop hardware sampling for all available hardware samplers")
#endif
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
        .def("as_yaml_string", &hws::system_hardware_sampler::as_yaml_string, "return all hardware samples for all hardware samplers as YAML string")
#if defined(HWS_MPI_SUPPORT_ENABLED)
        .def("dump_yaml_global", [](const hws::system_hardware_sampler &self, const std::string &filename, py::object py_comm) {
                const MPI_Comm comm = mpi_comm_from_python(py_comm);
                self.dump_yaml_global(filename, comm); }, py::arg("filename"), py::arg("comm"), "Let MPI rank 0 dump the hardware samples of all hardware samplers of all MPI ranks to the given YAML file using the provided mpi4py communicator.")
#endif
        .def("__repr__", [](const hws::system_hardware_sampler &self) { return fmt::format("<hws.SystemHardwareSampler with {} samples>", self.num_samplers()); });
}
