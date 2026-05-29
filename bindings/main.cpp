/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hws/version.hpp" // hws::version::version

#include "pybind11/pybind11.h"  // PYBIND11_MODULE, py::module_

#include <string_view>  // std::string_view

#if defined(HWS_MPI_SUPPORT_ENABLED)
#include <mpi.h>
#include <mpi4py/mpi4py.h>
#include "mpi4py_communicator.hpp"
#endif

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
    m.attr("__version__") = hws::version::version;

    // MPI support
#if defined(HWS_MPI_SUPPORT_ENABLED)
    // Initialize mpi4py C-API so PyMPIComm_* are usable
    if (import_mpi4py() < 0) {
        throw py::error_already_set();
    }
#endif
    m.def("has_mpi_support", []() { return HWS_IS_DEFINED(HWS_MPI_SUPPORT_ENABLED); });

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



#if defined(HWS_MPI_SUPPORT_ENABLED)
/**
 * Extracts an MPI_Comm from a python mpi4py.MPI.Comm object.
 * Has to be in same translation unit as the import_mpi4py() call to ensure that the mpi4py C-API is initialized and the PyMPIComm_Type is available.
 *
 * @param py_comm a Python object that is expected to be an mpi4py.MPI.Comm instance
 * @return the extracted MPI_Comm
 */
MPI_Comm mpi_comm_from_python(py::object py_comm) {
    if (!PyObject_TypeCheck(py_comm.ptr(), &PyMPIComm_Type)) {
        throw std::runtime_error("expected mpi4py.MPI.Comm as communicator argument");
    }

    MPI_Comm *comm_ptr = PyMPIComm_Get(py_comm.ptr());
    if (comm_ptr == nullptr) {
        throw std::runtime_error("could not extract MPI_Comm from mpi4py communicator");
    }

    return *comm_ptr;
}
#endif