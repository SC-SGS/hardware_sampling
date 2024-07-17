/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hardware_sampling/hardware_sampler.hpp"  // hws::hardware_sampler

#if defined(HWS_FOR_CPUS_ENABLED)
    #include "hardware_sampling/cpu/hardware_sampler.hpp"  // hws::cpu_hardware_sampler
#endif
#if defined(HWS_FOR_AMD_GPUS_ENABLED)
    #include "hardware_sampling/gpu_amd/hardware_sampler.hpp"  // hws::gpu_amd_hardware_sampler
#endif
#if defined(HWS_FOR_INTEL_GPUS_ENABLED)
    #include "hardware_sampling/gpu_intel/hardware_sampler.hpp"  // hws::gpu_intel_hardware_sampler
#endif
#if defined(HWS_FOR_NVIDIA_GPUS_ENABLED)
    #include "hardware_sampling/gpu_nvidia/hardware_sampler.hpp"  // hws::gpu_nvidia_hardware_sampler
#endif

#include "pybind11/pybind11.h"  // py::module_, py::class_

#include <format>  // std::format

namespace py = pybind11;

void init_hardware_sampler(py::module_ &m) {
    const py::module_ pure_virtual_module = m.def_submodule("__pure_virtual");

    // use a detail.tracking submodule for the hardware sampling
    py::module_ detail_module = m.def_submodule("detail", "a module containing detail functionality");
    const py::module_ tracking_module = detail_module.def_submodule("tracking", "a module containing performance tracking and hardware sampling functionality");

    // bind the pure virtual hardware sampler base class
    py::class_<hws::hardware_sampler> pyhardware_sampler(pure_virtual_module, "__pure_virtual_base_HardwareSampler");
    pyhardware_sampler.def("start", &hws::hardware_sampler::start_sampling, "start the current hardware sampling")
        .def("stop", &hws::hardware_sampler::stop_sampling, "stop the current hardware sampling")
        .def("pause", &hws::hardware_sampler::pause_sampling, "pause the current hardware sampling")
        .def("resume", &hws::hardware_sampler::resume_sampling, "resume the current hardware sampling")
        .def("has_started", &hws::hardware_sampler::has_sampling_started, "check whether hardware sampling has already been started")
        .def("is_sampling", &hws::hardware_sampler::is_sampling, "check whether the hardware sampling is currently active")
        .def("has_stopped", &hws::hardware_sampler::has_sampling_stopped, "check whether hardware sampling has already been stopped")
        .def("time_points", &hws::hardware_sampler::time_points, "get the time points of the respective hardware samples")
        .def("sampling_interval", &hws::hardware_sampler::sampling_interval, "get the sampling interval of this hardware sampler (in ms)")
        .def("__repr__", [](const hws::hardware_sampler &self) {
#if defined(HWS_FOR_CPUS_ENABLED)
            if (dynamic_cast<const hws::cpu_hardware_sampler *>(&self)) {
                return std::format("<plssvm.detail.tracking.CpuHardwareSampler with\n{}\n>", dynamic_cast<const hws::cpu_hardware_sampler &>(self));
            }
#endif
#if defined(HWS_FOR_NVIDIA_GPUS_ENABLED)
            if (dynamic_cast<const hws::gpu_nvidia_hardware_sampler *>(&self)) {
                return std::format("<plssvm.detail.tracking.GpuNvidiaHardwareSampler with\n{}\n>", dynamic_cast<const hws::gpu_nvidia_hardware_sampler &>(self));
            }
#endif
#if defined(HWS_FOR_AMD_GPUS_ENABLED)
            if (dynamic_cast<const hws::gpu_amd_hardware_sampler *>(&self)) {
                return std::format("<plssvm.detail.tracking.GpuAmdHardwareSampler with\n{}\n>", dynamic_cast<const hws::gpu_amd_hardware_sampler &>(self));
            }
#endif
#if defined(HWS_FOR_INTEL_GPUS_ENABLED)
            if (dynamic_cast<const hws::gpu_intel_hardware_sampler *>(&self)) {
                return std::format("<plssvm.detail.tracking.GpuIntelHardwareSampler with\n{}\n>", dynamic_cast<const hws::gpu_intel_hardware_sampler &>(self));
            }
#endif
        });
}
