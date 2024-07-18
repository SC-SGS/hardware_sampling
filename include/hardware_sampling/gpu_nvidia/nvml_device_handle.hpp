/**
 * @file
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Defines a pImpl class for an NVML device handle.
 */

#ifndef HARDWARE_SAMPLING_GPU_NVIDIA_NVML_DEVICE_HANDLE_HPP_
#define HARDWARE_SAMPLING_GPU_NVIDIA_NVML_DEVICE_HANDLE_HPP_
#pragma once

#include <cstddef>    // std::size_t
#include <memory>     // std::shared_ptr
#include <stdexcept>  // std::runtime_error

namespace hws::detail {

/**
 * @brief PImpl class to hide the nvmlDevice_t handle from the public interface (and, therefore, the "nvml.h" header).
 */
class nvml_device_handle {
  public:
    /**
     * @brief Default construct an nvml_device_handle. Such a handle may not be used in an NVML function!
     */
    nvml_device_handle() = default;
    /**
     * @brief Construct an nvml_device_handle for the device with ID @p device_id.
     * @param[in] device_id the device to get the handle for
     */
    explicit nvml_device_handle(std::size_t device_id);

    /**
     * @brief The pImpl helper struct.
     */
    struct nvml_device_handle_impl;

    /**
     * @brief Get the nvml_device_handle implementation used to access the actual nvmlDevice_t.
     * @throws hardware_sampling_exception if `*this` has been default constructed
     * @return the device handle (`[[nodiscard]]`)
     */
    [[nodiscard]] nvml_device_handle_impl &get_impl() {
        if (impl == nullptr) {
            throw std::runtime_error{ "Pointer to implementation is a nullptr! Maybe *this is default constructed?" };
        }
        return *impl;
    }

    /**
     * @brief Get the nvml_device_handle implementation used to access the actual nvmlDevice_t.
     * @throws hardware_sampling_exception if `*this` has been default constructed
     * @return the device handle (`[[nodiscard]]`)
     */
    [[nodiscard]] const nvml_device_handle_impl &get_impl() const {
        if (impl == nullptr) {
            throw std::runtime_error{ "Pointer to implementation is a nullptr! Maybe *this is default constructed?" };
        }
        return *impl;
    }

  private:
    /// The pointer to the actual implementation struct.
    std::shared_ptr<nvml_device_handle_impl> impl{};
};

}  // namespace hws::detail

#endif  // HARDWARE_SAMPLING_GPU_NVIDIA_NVML_DEVICE_HANDLE_HPP_
