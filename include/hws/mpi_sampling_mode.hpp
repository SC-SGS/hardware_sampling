/**
 * @file
 * @author Tim Thüring
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Defines the MPI sampling mode.
 */

#ifndef HWS_MPI_SAMPLING_MODE_HPP_
#define HWS_MPI_SAMPLING_MODE_HPP_
#pragma once

#if defined(HWS_MPI_SUPPORT_ENABLED)

namespace hws::detail {

/**
 * @brief The mode to use for MPI sampling.
 * per_rank: each rank creates hardware samplers for all devices visible to that rank
 * whole_node: if the same device is visible to more than one rank, only one of those ranks creates a hardware sampler for that device
 */
enum class mpi_sampling_mode {
    per_rank,
    whole_node
};

}  // namespace hws::detail

#endif  // HWS_MPI_SUPPORT_ENABLED

#endif  // HWS_MPI_SAMPLING_MODE_HPP_
