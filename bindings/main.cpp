/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "pybind11/pybind11.h"  // PYBIND11_MODULE, py::module_

#include <string_view>  // std::string_view

#define HWS_IS_DEFINED_HELPER(x) #x
#define HWS_IS_DEFINED(x) (std::string_view{ #x } != std::string_view{ HWS_IS_DEFINED_HELPER(x) })

namespace py = pybind11;

// forward declare binding functions
void init_event(py::module_ &);
void init_sample_category(py::module_ &);
void init_relative_event(py::module_ &);
void init_hardware_sampler(py::module_ &);
void init_system_hardware_sampler(py::module_ &);
void init_cpu_hardware_sampler(py::module_ &);
void init_gpu_nvidia_hardware_sampler(py::module_ &);
void init_gpu_amd_hardware_sampler(py::module_ &);
void init_gpu_intel_hardware_sampler(py::module_ &);
void init_version(py::module_ &);

PYBIND11_MODULE(HardwareSampling, m) {
    m.doc() = "Hardware Sampling for CPUs and GPUs";

    init_event(m);
    init_sample_category(m);
    init_relative_event(m);
    init_hardware_sampler(m);
    init_system_hardware_sampler(m);

    // CPU sampling
#if defined(HWS_FOR_CPUS_ENABLED)
    init_cpu_hardware_sampler(m);
#endif
    m.def("has_cpu_hardware_sampler", []() { return HWS_IS_DEFINED(HWS_FOR_CPUS_ENABLED); });

    // NVIDIA GPU sampling
#if defined(HWS_FOR_NVIDIA_GPUS_ENABLED)
    init_gpu_nvidia_hardware_sampler(m);
#endif
    m.def("has_gpu_nvidia_hardware_sampler", []() { return HWS_IS_DEFINED(HWS_FOR_NVIDIA_GPUS_ENABLED); });

    // AMD GPU sampling
#if defined(HWS_FOR_AMD_GPUS_ENABLED)
    init_gpu_amd_hardware_sampler(m);
#endif
    m.def("has_gpu_amd_hardware_sampler", []() { return HWS_IS_DEFINED(HWS_FOR_AMD_GPUS_ENABLED); });

    // Intel GPU sampling
#if defined(HWS_FOR_INTEL_GPUS_ENABLED)
    init_gpu_intel_hardware_sampler(m);
#endif
    m.def("has_gpu_intel_hardware_sampler", []() { return HWS_IS_DEFINED(HWS_FOR_INTEL_GPUS_ENABLED); });

    init_version(m);
}
