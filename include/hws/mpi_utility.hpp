/**
 * @file
 * @author Tim Thüring
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief MPI utility functions for hardware sampling.
 */

#ifndef HWS_MPI_UTILITY_HPP_
#define HWS_MPI_UTILITY_HPP_
#pragma once

#if defined(HWS_MPI_SUPPORT_ENABLED)

    #include "hws/visible_gpu_device.hpp"  // hws::detail::visible_gpu_device

    #include <mpi.h>   // MPI_Comm, MPI_COMM_NULL
    #include <string>  // std::string
    #include <vector>  // std::vector

namespace hws::detail {

/**
 * @brief Gather YAML strings from all MPI ranks and assemble them in rank order on rank 0.
 *
 * @param[in] local_yaml the local YAML string contribution
 * @param[in] communicator the MPI communicator
 *
 * @return concatenated YAML string on rank 0, empty string on all other ranks
 */
[[nodiscard]] std::string gather_yaml_strings_mpi(const std::string &local_yaml, MPI_Comm communicator);

/**
 * @brief RAII wrapper around a node-local MPI communicator for whole-node sampling.
 */
struct hostname_comm_info {
    MPI_Comm node_comm = MPI_COMM_NULL;
    int node_rank = 0;
    int node_size = 1;

    explicit hostname_comm_info(MPI_Comm comm);

    hostname_comm_info(const hostname_comm_info &) = delete;
    hostname_comm_info &operator=(const hostname_comm_info &) = delete;

    ~hostname_comm_info();
};

/**
 * Computes for each MPI rank a list of devices that have to be sampled by this rank. Ensures that
 * each device is sampled by exactly one rank.
 *
 * @param local_devices a vector of visible_gpu_device for the local rank, each containing a local index and a physical ID
 * @param node_comm a node local MPI communicator
 * @return all device indices that have to be sampled by this rank
 */
[[nodiscard]] std::vector<int> owned_local_indices_for_backend(const std::vector<visible_gpu_device> &local_devices, MPI_Comm node_comm);

}  // namespace hws::detail

#endif  // HWS_MPI_SUPPORT_ENABLED

#endif  // HWS_MPI_UTILITY_HPP_
