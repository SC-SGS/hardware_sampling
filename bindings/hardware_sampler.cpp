/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hardware_sampling/hardware_sampler.hpp"  // hws::hardware_sampler

#include "hardware_sampling/event.hpp"    // hws::event
#include "hardware_sampling/utility.hpp"  // hws::detail::durations_from_reference_time

#if defined(HWS_FOR_CPUS_ENABLED)
    #include "hardware_sampling/cpu/hardware_sampler.hpp"  // hws::cpu_hardware_sampler
#endif
#if defined(HWS_FOR_NVIDIA_GPUS_ENABLED)
    #include "hardware_sampling/gpu_nvidia/hardware_sampler.hpp"  // hws::gpu_nvidia_hardware_sampler
#endif
#if defined(HWS_FOR_AMD_GPUS_ENABLED)
    #include "hardware_sampling/gpu_amd/hardware_sampler.hpp"  // hws::gpu_amd_hardware_sampler
#endif
#if defined(HWS_FOR_INTEL_GPUS_ENABLED)
    #include "hardware_sampling/gpu_intel/hardware_sampler.hpp"  // hws::gpu_intel_hardware_sampler
#endif

#include "fmt/format.h"         // fmt::format
#include "pybind11/chrono.h"    // bind std::chrono types
#include "pybind11/pybind11.h"  // py::module_, py::class_
#include "pybind11/stl.h"       // bind STL types

#include <string>  // std::string

namespace hws {

/**
 * @brief A struct encapsulating a single event with a relative time point.
 */
struct relative_event {
    /**
     * @brief Construct a new event given a time point and name.
     * @param[in] time_point_p the time when the event occurred relative to the first event
     * @param[in] name_p the name of the event
     */
    relative_event(const double relative_time_point_p, std::string name_p) :
        relative_time_point{ relative_time_point_p },
        name{ std::move(name_p) } { }

    /// The relative time point this event occurred at.
    double relative_time_point;
    /// The name of this event.
    std::string name;
};

}  // namespace hws

namespace py = pybind11;

void init_hardware_sampler(py::module_ &m) {
    const py::module_ pure_virtual_module = m.def_submodule("__pure_virtual");

    // a special python only struct encapsulating a relative event, i.e., an event where its "relative_time_point" member is the time passed since the first event
    py::class_<hws::relative_event>(m, "RelativeEvent")
        .def(py::init<decltype(hws::relative_event::relative_time_point), decltype(hws::relative_event::name)>(), "construct a new event using a time point and a name")
        .def_readonly("relative_time_point", &hws::relative_event::relative_time_point, "read the relative time point associated to this event")
        .def_readonly("name", &hws::relative_event::name, "read the name associated to this event")
        .def("__repr__", [](const hws::relative_event &self) {
            return fmt::format("<HardWareSampling.RelativeEvent with {{ time_point: {}, name: {} }}>", self.relative_time_point, self.name);
        });

    // bind the pure virtual hardware sampler base class
    py::class_<hws::hardware_sampler> pyhardware_sampler(pure_virtual_module, "__pure_virtual_base_HardwareSampler");
    pyhardware_sampler.def("start", &hws::hardware_sampler::start_sampling, "start the current hardware sampling")
        .def("stop", &hws::hardware_sampler::stop_sampling, "stop the current hardware sampling")
        .def("pause", &hws::hardware_sampler::pause_sampling, "pause the current hardware sampling")
        .def("resume", &hws::hardware_sampler::resume_sampling, "resume the current hardware sampling")
        .def("has_started", &hws::hardware_sampler::has_sampling_started, "check whether hardware sampling has already been started")
        .def("is_sampling", &hws::hardware_sampler::is_sampling, "check whether the hardware sampling is currently active")
        .def("has_stopped", &hws::hardware_sampler::has_sampling_stopped, "check whether hardware sampling has already been stopped")
        .def("add_event", py::overload_cast<hws::event>(&hws::hardware_sampler::add_event), "add a new event")
        .def("add_event", py::overload_cast<decltype(hws::event::time_point), decltype(hws::event::name)>(&hws::hardware_sampler::add_event), "add a new event using a time point and a name")
        .def("add_event", py::overload_cast<decltype(hws::event::name)>(&hws::hardware_sampler::add_event), "add a new event using a name, the current time is used as time point")
        .def("num_events", &hws::hardware_sampler::num_events, "get the number of events")
        .def("get_events", &hws::hardware_sampler::get_events, "get all events")
        .def("get_relative_events", [](const hws::hardware_sampler &self) {
            std::vector<hws::relative_event> relative_events{};
            for (const hws::event &e : self.get_events()) {
                relative_events.emplace_back(hws::detail::duration_from_reference_time(e.time_point, self.get_event(0).time_point), e.name);
            }
            return relative_events; }, "get all relative events")
        .def("get_event", &hws::hardware_sampler::get_event, "get a specific event")
        .def("get_relative_event", [](const hws::hardware_sampler &self, const std::size_t idx) { return hws::relative_event{ hws::detail::duration_from_reference_time(self.get_event(idx).time_point, self.get_event(0).time_point), self.get_event(idx).name }; }, "get a specific relative event")
        .def("time_points", &hws::hardware_sampler::sampling_time_points, "get the time points of the respective hardware samples")
        .def("relative_time_points", [](const hws::hardware_sampler &self) { return hws::detail::durations_from_reference_time(self.sampling_time_points(), self.get_event(0).time_point); }, "get the relative durations of the respective hardware samples in seconds (as \"normal\" number)")
        .def("sampling_interval", &hws::hardware_sampler::sampling_interval, "get the sampling interval of this hardware sampler (in ms)")
        .def("dump_yaml", py::overload_cast<const std::string &>(&hws::hardware_sampler::dump_yaml, py::const_), "dump all hardware samples to the given YAML file")
        .def("__repr__", [](const hws::hardware_sampler &self) {
#if defined(HWS_FOR_CPUS_ENABLED)
            if (dynamic_cast<const hws::cpu_hardware_sampler *>(&self)) {
                return fmt::format("<plssvm.detail.tracking.CpuHardwareSampler with\n{}\n>", dynamic_cast<const hws::cpu_hardware_sampler &>(self));
            }
#endif
#if defined(HWS_FOR_NVIDIA_GPUS_ENABLED)
            if (dynamic_cast<const hws::gpu_nvidia_hardware_sampler *>(&self)) {
                return fmt::format("<plssvm.detail.tracking.GpuNvidiaHardwareSampler with\n{}\n>", dynamic_cast<const hws::gpu_nvidia_hardware_sampler &>(self));
            }
#endif
#if defined(HWS_FOR_AMD_GPUS_ENABLED)
            if (dynamic_cast<const hws::gpu_amd_hardware_sampler *>(&self)) {
                return fmt::format("<plssvm.detail.tracking.GpuAmdHardwareSampler with\n{}\n>", dynamic_cast<const hws::gpu_amd_hardware_sampler &>(self));
            }
#endif
#if defined(HWS_FOR_INTEL_GPUS_ENABLED)
            if (dynamic_cast<const hws::gpu_intel_hardware_sampler *>(&self)) {
                return fmt::format("<plssvm.detail.tracking.GpuIntelHardwareSampler with\n{}\n>", dynamic_cast<const hws::gpu_intel_hardware_sampler &>(self));
            }
#endif
            return std::string{ "unknown" }; });
}
