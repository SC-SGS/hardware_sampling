/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hardware_sampling/system_hardware_sampler.hpp"

#include "hardware_sampling/event.hpp"  // hws::event

#if defined(HWS_FOR_CPUS_ENABLED)
    #include "hardware_sampling/cpu/hardware_sampler.hpp"  // hws::cpu_hardware_sampler
#endif
#if defined(HWS_FOR_NVIDIA_GPUS_ENABLED)
    #include "hardware_sampling/gpu_nvidia/hardware_sampler.hpp"  // hws::gpu_nvidia_hardware_sampler
    #include "hardware_sampling/gpu_nvidia/utility.hpp"           // HWS_CUDA_ERROR_CHECK, hws::detail::
#endif
#if defined(HWS_FOR_AMD_GPUS_ENABLED)
    #include "hardware_sampling/gpu_amd/hardware_sampler.hpp"  // hws::gpu_amd_hardware_sampler
    #include "hardware_sampling/gpu_amd/utility.hpp"           // HWS_HIP_ERROR_CHECK

    #include "hip/hip_runtime.h"  // hipGetDeviceCount
#endif
#if defined(HWS_FOR_INTEL_GPUS_ENABLED)
    #include "hardware_sampling/gpu_intel/hardware_sampler.hpp"  // hws::gpu_intel_hardware_sampler
    #include "hardware_sampling/gpu_intel/utility.hpp"           // HWS_LEVEL_ZERO_ERROR_CHECK
#endif

#include "fmt/format.h"  // fmt::format

#include <algorithm>  // std::for_each, std::all_of
#include <chrono>     // std::chrono::milliseconds
#include <memory>     // std::unique_ptr, std::make_unique
#include <stdexcept>  // std::out_of_range
#include <vector>     // std::vector

namespace hws {

system_hardware_sampler::system_hardware_sampler() :
    system_hardware_sampler{ HWS_SAMPLING_INTERVAL } { }

system_hardware_sampler::system_hardware_sampler(const std::chrono::milliseconds sampling_interval) {
    // create the hardware samplers based on the available hardware
#if defined(HWS_FOR_CPUS_ENABLED)
    {
        samplers_.push_back(std::make_unique<cpu_hardware_sampler>(sampling_interval));
    }
#endif
#if defined(HWS_FOR_NVIDIA_GPUS_ENABLED)
    {
        int device_count{};
        HWS_CUDA_ERROR_CHECK(cudaGetDeviceCount(&device_count));
        for (int device = 0; device < device_count; ++device) {
            samplers_.push_back(std::make_unique<gpu_nvidia_hardware_sampler>(static_cast<std::size_t>(device), sampling_interval));
        }
    }
#endif
#if defined(HWS_FOR_AMD_GPUS_ENABLED)
    {
        int device_count{};
        HWS_HIP_ERROR_CHECK(hipGetDeviceCount(&device_count));
        for (int device = 0; device < device_count; ++device) {
            samplers_.push_back(std::make_unique<gpu_amd_hardware_sampler>(static_cast<std::size_t>(device), sampling_interval));
        }
    }
#endif
#if defined(HWS_FOR_INTEL_GPUS_ENABLED)
    {
        // TODO: implement
    }
#endif
}

void system_hardware_sampler::start_sampling() {
    std::for_each(samplers_.begin(), samplers_.end(), [](auto &ptr) { ptr->start_sampling(); });
}

void system_hardware_sampler::stop_sampling() {
    std::for_each(samplers_.begin(), samplers_.end(), [](auto &ptr) { ptr->stop_sampling(); });
}

void system_hardware_sampler::pause_sampling() {
    std::for_each(samplers_.begin(), samplers_.end(), [](auto &ptr) { ptr->pause_sampling(); });
}

void system_hardware_sampler::resume_sampling() {
    std::for_each(samplers_.begin(), samplers_.end(), [](auto &ptr) { ptr->resume_sampling(); });
}

bool system_hardware_sampler::has_sampling_started() const noexcept {
    return std::all_of(samplers_.cbegin(), samplers_.cend(), [](const auto &ptr) { return ptr->has_sampling_started(); });
}

bool system_hardware_sampler::is_sampling() const noexcept {
    return std::all_of(samplers_.cbegin(), samplers_.cend(), [](const auto &ptr) { return ptr->is_sampling(); });
}

bool system_hardware_sampler::has_sampling_stopped() const noexcept {
    return std::all_of(samplers_.cbegin(), samplers_.cend(), [](const auto &ptr) { return ptr->has_sampling_stopped(); });
}

void system_hardware_sampler::add_event(event e) {
    std::for_each(samplers_.begin(), samplers_.end(), [&e](auto &ptr) { ptr->add_event(e); });
}

void system_hardware_sampler::add_event(decltype(event::time_point) time_point, decltype(event::name) name) {
    std::for_each(samplers_.begin(), samplers_.end(), [&time_point, &name](auto &ptr) { ptr->add_event(time_point, name); });
}

void system_hardware_sampler::add_event(decltype(event::name) name) {
    std::for_each(samplers_.begin(), samplers_.end(), [&name](auto &ptr) { ptr->add_event(name); });
}

std::vector<std::size_t> system_hardware_sampler::num_events() const {
    std::vector<std::size_t> num_events_per_sampler(this->num_samplers());
    std::transform(samplers_.cbegin(), samplers_.cend(), num_events_per_sampler.begin(), [](const auto &ptr) { return ptr->num_events(); });
    return num_events_per_sampler;
}

std::vector<std::vector<event>> system_hardware_sampler::get_events() const {
    std::vector<std::vector<event>> events_per_sampler(this->num_samplers());
    std::transform(samplers_.cbegin(), samplers_.cend(), events_per_sampler.begin(), [](const auto &ptr) { return ptr->get_events(); });
    return events_per_sampler;
}

std::vector<std::vector<std::chrono::steady_clock::time_point>> system_hardware_sampler::sampling_time_points() const {
    std::vector<std::vector<std::chrono::steady_clock::time_point>> sampling_time_points_per_sampler(this->num_samplers());
    std::transform(samplers_.cbegin(), samplers_.cend(), sampling_time_points_per_sampler.begin(), [](const auto &ptr) { return ptr->sampling_time_points(); });
    return sampling_time_points_per_sampler;
}

std::vector<std::chrono::milliseconds> system_hardware_sampler::sampling_interval() const {
    std::vector<std::chrono::milliseconds> sampling_interval_per_sampler(this->num_samplers());
    std::transform(samplers_.cbegin(), samplers_.cend(), sampling_interval_per_sampler.begin(), [](const auto &ptr) { return ptr->sampling_interval(); });
    return sampling_interval_per_sampler;
}

std::size_t system_hardware_sampler::num_samplers() const noexcept {
    return samplers_.size();
}

std::vector<std::unique_ptr<hardware_sampler>> &system_hardware_sampler::samplers() noexcept {
    return samplers_;
}

const std::vector<std::unique_ptr<hardware_sampler>> &system_hardware_sampler::samplers() const noexcept {
    return samplers_;
}

std::unique_ptr<hardware_sampler> &system_hardware_sampler::sampler(const std::size_t idx) {
    if (idx >= samplers_.size()) {
        throw std::out_of_range{ fmt::format("Index {} is out-of-range for size {}!", idx, samplers_.size()) };
    }
    return samplers_[idx];
}

const std::unique_ptr<hardware_sampler> &system_hardware_sampler::sampler(const std::size_t idx) const {
    if (idx >= samplers_.size()) {
        throw std::out_of_range{ fmt::format("Index {} is out-of-range for size {}!", idx, samplers_.size()) };
    }
    return samplers_[idx];
}

void system_hardware_sampler::dump_yaml(const char *filename) const {
    std::for_each(samplers_.cbegin(), samplers_.cend(), [&filename](const auto &ptr) { ptr->dump_yaml(filename); });
}

void system_hardware_sampler::dump_yaml(const std::string &filename) const {
    std::for_each(samplers_.cbegin(), samplers_.cend(), [&filename](const auto &ptr) { ptr->dump_yaml(filename); });
}

void system_hardware_sampler::dump_yaml(const std::filesystem::path &filename) const {
    std::for_each(samplers_.cbegin(), samplers_.cend(), [&filename](const auto &ptr) { ptr->dump_yaml(filename); });
}

}  // namespace hws