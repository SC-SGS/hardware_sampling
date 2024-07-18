/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "pybind11/pybind11.h"  // PYBIND11_MODULE, py::module_

namespace py = pybind11;

// forward declare binding functions
void init_event(py::module_ &);
void init_hardware_sampler(py::module_ &);
void init_cpu_hardware_sampler(py::module_ &);
void init_gpu_nvidia_hardware_sampler(py::module_ &);
void init_gpu_amd_hardware_sampler(py::module_ &);
void init_gpu_intel_hardware_sampler(py::module_ &);

PYBIND11_MODULE(py_hardware_sampling, m) {
    m.doc() = "Hardware Sampling for CPUs and GPUs";

    init_event(m);
    init_hardware_sampler(m);

#if defined(HWS_FOR_CPUS_ENABLED)
    init_cpu_hardware_sampler(m);
#endif
#if defined(HWS_FOR_NVIDIA_GPUS_ENABLED)
    init_gpu_nvidia_hardware_sampler(m);
#endif
#if defined(HWS_FOR_AMD_GPUS_ENABLED)
    init_gpu_amd_hardware_sampler(m);
#endif
#if defined(HWS_FOR_INTEL_GPUS_ENABLED)
    init_gpu_intel_hardware_sampler(m);
#endif
}
