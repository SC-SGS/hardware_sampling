/**
 * @file
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Implements a pImpl class for an NVML device handle.
 */

#ifndef HARDWARE_SAMPLING_GPU_NVIDIA_NVML_DEVICE_HANDLE_IMPL_HPP_
#define HARDWARE_SAMPLING_GPU_NVIDIA_NVML_DEVICE_HANDLE_IMPL_HPP_
#pragma once

#include "hardware_sampling/gpu_nvidia/nvml_device_handle.hpp"  // hws::detail::nvml_device_handle
#include "hardware_sampling/gpu_nvidia/utility.hpp"             // HWS_NVML_ERROR_CHECK

#include "nvml.h"  // nvmlDevice_t

#include <cstddef>  // std::size_t
#include <memory>   // std::make_shared

namespace hws::detail {

/**
 * @brief The PImpl implementation struct encapsulating a nvmlDevice_t.
 */
struct nvml_device_handle::nvml_device_handle_impl {
  public:
    /**
     * @brief Get the nvmlDevice_t for the device with ID @p device_id.
     * @param[in] device_id the device to get the handle for
     */
    explicit nvml_device_handle_impl(const std::size_t device_id) {
        HWS_NVML_ERROR_CHECK(nvmlDeviceGetHandleByIndex(static_cast<int>(device_id), &device))
    }

    /// The wrapped NVML device handle.
    nvmlDevice_t device{};
};

inline nvml_device_handle::nvml_device_handle(const std::size_t device_id) :
    impl{ std::make_shared<nvml_device_handle_impl>(device_id) } { }

}  // namespace hws::detail

#endif  // HARDWARE_SAMPLING_GPU_NVIDIA_NVML_DEVICE_HANDLE_IMPL_HPP_
