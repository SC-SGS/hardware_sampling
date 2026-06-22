/**
 * @file
 * @author Tim Thüring
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Utility functions for transforming mpi4py communicators into C++ MPI communicators
 */

#ifndef HWS_BINDINGS_MPI4PY_COMMUNICATOR_HPP
#define HWS_BINDINGS_MPI4PY_COMMUNICATOR_HPP
#pragma once

#include "pybind11/pybind11.h"

#if defined(HWS_MPI_SUPPORT_ENABLED)
    #include <mpi.h>
#endif

namespace py = pybind11;

#if defined(HWS_MPI_SUPPORT_ENABLED)
MPI_Comm mpi_comm_from_python(py::object py_comm);
#endif

#endif  // HWS_BINDINGS_MPI4PY_COMMUNICATOR_HPP
