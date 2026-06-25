/**
 * @file
 * @author Tim Thüring
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Defines types for identifying visible GPU devices.
 */

#ifndef HWS_VISIBLE_GPU_DEVICE_HPP_
#define HWS_VISIBLE_GPU_DEVICE_HPP_
#pragma once

#if defined(HWS_MPI_SUPPORT_ENABLED)

#include <string>  // std::string

namespace hws::detail {

/**
 * @brief Enum class representing the backend kind of visible GPU device.
 * @details The backend kind can be NVIDIA, AMD, or Intel.
 */
enum class device_backend_kind {
    nvidia,
    amd,
    intel
};

/**
 * @brief Represents a visible GPU device on the local rank.
 * @details Contains the backend kind, the local index of the device for that backend on this rank, and a stable per-node identifier (physical ID) for the device.
 */
struct visible_gpu_device {
    device_backend_kind backend;
    int local_index;          // device index for that backend on this rank
    std::string physical_id;  // stable per-node identifier
};

}  // namespace hws::detail

#endif  // HWS_MPI_SUPPORT_ENABLED

#endif  // HWS_VISIBLE_GPU_DEVICE_HPP_
