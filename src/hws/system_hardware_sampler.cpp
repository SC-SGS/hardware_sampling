/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hws/system_hardware_sampler.hpp"

#include "hws/event.hpp"            // hws::event
#include "hws/sample_category.hpp"  // hws::sample_category

#if defined(HWS_FOR_CPUS_ENABLED)
    #include "hws/cpu/hardware_sampler.hpp"  // hws::cpu_hardware_sampler
#endif
#if defined(HWS_FOR_CRAY_PM_COUNTERS_ENABLED)
    #include "hws/cray_pm_counters/hardware_sampler.hpp"  // hws::cray_pm_counters_hardware_sampler
    #include "hws/cray_pm_counters/utility.hpp"            // hws::detail::pm_counters_available
#endif
#if defined(HWS_FOR_NVIDIA_GPUS_ENABLED)
    #include "hws/gpu_nvidia/hardware_sampler.hpp"  // hws::gpu_nvidia_hardware_sampler
    #include "hws/gpu_nvidia/utility.hpp"           // HWS_CUDA_ERROR_CHECK

    #include "cuda_runtime.h"  // cudaGetDeviceCount
#endif
#if defined(HWS_FOR_AMD_GPUS_ENABLED)
    #include "hws/gpu_amd/hardware_sampler.hpp"  // hws::gpu_amd_hardware_sampler
    #include "hws/gpu_amd/utility.hpp"           // HWS_HIP_ERROR_CHECK

    #include "hip/hip_runtime.h"  // hipGetDeviceCount
#endif
#if defined(HWS_FOR_INTEL_GPUS_ENABLED)
    #include "hws/gpu_intel/hardware_sampler.hpp"  // hws::gpu_intel_hardware_sampler
    #include "hws/gpu_intel/utility.hpp"           // HWS_LEVEL_ZERO_ERROR_CHECK
#endif

#include "fmt/format.h"  // fmt::format
#include "fmt/ranges.h"  // fmt::join

#include <algorithm>  // std::for_each, std::all_of
#include <chrono>     // std::chrono::milliseconds
#include <cstddef>    // std::size_t
#include <cstdint>    // std::uint32_t
#include <fstream>    // std::ofstream
#include <memory>     // std::unique_ptr, std::make_unique
#include <numeric>    // std::accumulate
#include <optional>   // std::optional
#include <stdexcept>  // std::out_of_range
#include <string>     // std::string, std::to_string
#include <utility>    // std::pair
#include <vector>     // std::vector

#if defined(HWS_MPI_SUPPORT_ENABLED)
    #include "hws/mpi_utility.hpp"  // hws::detail::hostname_comm_info, hws::detail::owned_local_indices_for_backend
#endif

namespace hws {

system_hardware_sampler::system_hardware_sampler(const sample_category category) :
    system_hardware_sampler{ HWS_SAMPLING_INTERVAL, category } { }

system_hardware_sampler::system_hardware_sampler(const std::chrono::milliseconds sampling_interval, sample_category category) {
    // create the hardware samplers based on the available hardware
    create_local_samplers(sampling_interval, category);
}

#if defined(HWS_MPI_SUPPORT_ENABLED)
system_hardware_sampler::system_hardware_sampler(MPI_Comm communicator, const detail::mpi_sampling_mode mode, const sample_category category) :
    system_hardware_sampler(communicator, mode, HWS_SAMPLING_INTERVAL, category) { }

system_hardware_sampler::system_hardware_sampler(MPI_Comm communicator, const detail::mpi_sampling_mode mode, const std::chrono::milliseconds sampling_interval, const sample_category category) {
    if (mode == detail::mpi_sampling_mode::per_rank) {
        // each rank creates samplers for all devices visible to him
        create_local_samplers(sampling_interval, category);
    } else if (mode == detail::mpi_sampling_mode::whole_node) {
        // create a custom, node-local MPI communicator
        detail::hostname_comm_info nc{ communicator };

        // CPU: one sampler per node --> node leader only
        #if defined(HWS_FOR_CPUS_ENABLED)
            if (nc.node_rank == 0) {
                samplers_.push_back(std::make_unique<cpu_hardware_sampler>(sampling_interval, category));
            }
        #endif

        // Cray pm_counters: one sampler per node --> node leader only
        #if defined(HWS_FOR_CRAY_PM_COUNTERS_ENABLED)
            if (nc.node_rank == 0 && detail::pm_counters_available()) {
                samplers_.push_back(std::make_unique<cray_pm_counters_hardware_sampler>(sampling_interval, category));
            }
        #endif

        // NVIDIA
        #if defined(HWS_FOR_NVIDIA_GPUS_ENABLED)
            {
                const std::vector<detail::visible_gpu_device> local = detail::enumerate_local_nvidia_devices();
                const std::vector<int> owned = detail::owned_local_indices_for_backend(local, nc.node_comm);
                for (int const idx : owned) {
                    samplers_.push_back(std::make_unique<gpu_nvidia_hardware_sampler>(static_cast<std::size_t>(idx), sampling_interval, category));
                }
            }
        #endif

        // AMD
        #if defined(HWS_FOR_AMD_GPUS_ENABLED)
            {
                const std::vector<detail::visible_gpu_device> local = detail::enumerate_local_amd_devices();
                const std::vector<int> owned = detail::owned_local_indices_for_backend(local, nc.node_comm);
                for (int const idx : owned) {
                    samplers_.push_back(std::make_unique<gpu_amd_hardware_sampler>(
                        static_cast<std::size_t>(idx), sampling_interval, category));
                }
            }
        #endif

        // Intel
        #if defined(HWS_FOR_INTEL_GPUS_ENABLED)
            {
                const std::vector<detail::visible_gpu_device> local = detail::enumerate_local_intel_devices();
                const std::vector<int> owned = detail::owned_local_indices_for_backend(local, nc.node_comm);
                for (int const idx : owned) {
                    samplers_.push_back(std::make_unique<gpu_intel_hardware_sampler>(static_cast<std::size_t>(idx), sampling_interval, category));
                }
            }
        #endif

    } else {
        throw std::runtime_error{ fmt::format("Unknown MPI sampling mode {}!", static_cast<int>(mode)) };
    }
}
#endif

void system_hardware_sampler::start_sampling() {
    std::for_each(samplers_.begin(), samplers_.end(), [](auto &ptr) { ptr->start_sampling(); });
}

#if defined(HWS_MPI_SUPPORT_ENABLED)
void system_hardware_sampler::start_sampling(MPI_Comm communicator) {
    MPI_Barrier(communicator);
    std::for_each(samplers_.begin(), samplers_.end(), [](auto &ptr) { ptr->start_sampling(); });
}
#endif
void system_hardware_sampler::stop_sampling() {
    std::for_each(samplers_.begin(), samplers_.end(), [](auto &ptr) { ptr->stop_sampling(); });
}

#if defined(HWS_MPI_SUPPORT_ENABLED)
void system_hardware_sampler::stop_sampling(MPI_Comm communicator) {
    std::for_each(samplers_.begin(), samplers_.end(), [](auto &ptr) { ptr->stop_sampling(); });
    MPI_Barrier(communicator);
}
#endif

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

    // each individual sampler already wrote its own "---"-separated YAML document above; the correlation hints
    // aren't tied to any single sampler, so they get one more such document of their own, if there's anything to
    // report (see device_correlation_hints_as_yaml_string()).
    const std::string hints = this->device_correlation_hints_as_yaml_string();
    if (!hints.empty()) {
        std::ofstream file{ filename, std::ios_base::app };
        file << "---\n\n"
             << hints;
    }
}

void system_hardware_sampler::dump_yaml(const std::string &filename) const {
    this->dump_yaml(filename.c_str());
}

void system_hardware_sampler::dump_yaml(const std::filesystem::path &filename) const {
    this->dump_yaml(filename.string().c_str());
}

#if defined(HWS_MPI_SUPPORT_ENABLED)
void system_hardware_sampler::dump_yaml_global(const char *filename, MPI_Comm communicator) const {
    int initialized = 0;
    MPI_Initialized(&initialized);

    if (!initialized) {
        throw std::runtime_error{"MPI must already be initialized"};
    }

    // MPI rank and world size for identification and communication
    int rank = 0;
    MPI_Comm_rank(communicator, &rank);

    std::string rank_yaml_output;  // yaml file as string per rank

    rank_yaml_output += "---\n\n";
    rank_yaml_output += "rank: " + std::to_string(rank) + "\n\n";

    // accumulate string from each sampler
    std::size_t sampler_idx = 0;
    std::for_each(samplers_.cbegin(), samplers_.cend(), [&rank_yaml_output, &sampler_idx](const auto &ptr) {
        rank_yaml_output += "sampler_" + std::to_string(sampler_idx++) + ":\n";
        rank_yaml_output += detail::indent_lines(ptr->as_yaml_string(), "  ");
    });

    // not tied to any single sampler, so appended directly rather than under a "sampler_N:" key; empty if there's
    // nothing to report (see device_correlation_hints_as_yaml_string())
    rank_yaml_output += this->device_correlation_hints_as_yaml_string();

    const std::string global_yaml_output = detail::gather_yaml_strings_mpi(rank_yaml_output, communicator);

    if (rank == 0) {
        std::ofstream file(filename);
        file << global_yaml_output;
    }
}

void system_hardware_sampler::dump_yaml_global(const std::string &filename, MPI_Comm communicator) const {
    this->dump_yaml_global(filename.c_str(), communicator);
}

void system_hardware_sampler::dump_yaml_global(const std::filesystem::path &filename, MPI_Comm communicator) const {
    this->dump_yaml_global(filename.string().c_str(), communicator);
}
#endif

std::string system_hardware_sampler::as_yaml_string() const {
    return std::accumulate(samplers_.cbegin(), samplers_.cend(), std::string{}, [](const std::string str, const auto &ptr) { return str + ptr->as_yaml_string(); })
           + this->device_correlation_hints_as_yaml_string();
}

std::string system_hardware_sampler::samples_only_as_yaml_string() const {
    return std::accumulate(samplers_.cbegin(), samplers_.cend(), std::string{}, [](const std::string str, const auto &ptr) { return str + ptr->samples_only_as_yaml_string(); });
}

#if defined(HWS_FOR_CRAY_PM_COUNTERS_ENABLED) && (defined(HWS_FOR_AMD_GPUS_ENABLED) || defined(HWS_FOR_NVIDIA_GPUS_ENABLED))
std::string system_hardware_sampler::device_correlation_hints_as_yaml_string() const {
    const cray_pm_counters_hardware_sampler *pm_sampler = nullptr;
    #if defined(HWS_FOR_AMD_GPUS_ENABLED)
    std::vector<std::pair<std::size_t, std::string>> amd_devices{};
    #endif
    #if defined(HWS_FOR_NVIDIA_GPUS_ENABLED)
    std::vector<std::pair<std::size_t, std::string>> nvidia_devices{};
    #endif
    for (const std::unique_ptr<hardware_sampler> &ptr : samplers_) {
        if (const auto *pm = dynamic_cast<const cray_pm_counters_hardware_sampler *>(ptr.get()); pm != nullptr) {
            pm_sampler = pm;
            continue;
        }
    #if defined(HWS_FOR_AMD_GPUS_ENABLED)
        if (const auto *amd = dynamic_cast<const gpu_amd_hardware_sampler *>(ptr.get()); amd != nullptr) {
            // hip_device_id() (the "Nth GPU visible to this process/rank") for local_index, but pci_bus_id()
            // (ROCm SMI, the same API family used for all of this sampler's actual measurements) for the PCI bus
            // ID - not hip_device_id()'s own HIP-space bus id, since ROCm SMI's and HIP's device enumerations can
            // diverge under HIP_VISIBLE_DEVICES/ROCR_VISIBLE_DEVICES and using a different API family than the
            // one the sampler measures with could silently attribute the wrong PCI bus ID.
            amd_devices.emplace_back(amd->hip_device_id(), amd->pci_bus_id());
            continue;
        }
    #endif
    #if defined(HWS_FOR_NVIDIA_GPUS_ENABLED)
        if (const auto *nvidia = dynamic_cast<const gpu_nvidia_hardware_sampler *>(ptr.get()); nvidia != nullptr) {
            // pci_bus_id() (NVML, the same API family used for all of this sampler's actual measurements) is used
            // here rather than CUDA's own cudaDeviceGetPCIBusId(), since NVML's device enumeration isn't affected
            // by CUDA_VISIBLE_DEVICES while the CUDA runtime's is - using a different API family than the one the
            // sampler measures with could silently attribute the wrong PCI bus ID.
            nvidia_devices.emplace_back(nvidia->device_id(), nvidia->pci_bus_id());
            continue;
        }
    #endif
    }

    const bool any_visible_gpus =
    #if defined(HWS_FOR_AMD_GPUS_ENABLED)
        !amd_devices.empty()
    #else
        false
    #endif
    #if defined(HWS_FOR_NVIDIA_GPUS_ENABLED)
        || !nvidia_devices.empty()
    #endif
        ;
    if (pm_sampler == nullptr || !any_visible_gpus) {
        return "";
    }

    // ground truth, no guessing involved: which accel[i] counters pm_counters exposed on this node
    const std::vector<int> accel_indices = pm_sampler->discovered_accel_indices();

    std::string vendor_blocks{};
    #if defined(HWS_FOR_AMD_GPUS_ENABLED)
    if (!amd_devices.empty()) {
        vendor_blocks += detail::accel_correlation_yaml_block("amd", amd_devices, detail::enumerate_all_amd_gpu_pci_bus_ids(), accel_indices);
    }
    #endif
    #if defined(HWS_FOR_NVIDIA_GPUS_ENABLED)
    if (!nvidia_devices.empty()) {
        vendor_blocks += detail::accel_correlation_yaml_block("nvidia", nvidia_devices, detail::enumerate_all_nvidia_gpu_pci_bus_ids(), accel_indices);
    }
    #endif

    return fmt::format("device_correlation_hints:\n"
                       "  note: \"UNVERIFIED heuristic: assumes Cray pm_counters numbers accel[i] in ascending PCI bus address order among all physically present GPUs of a given vendor; this is not confirmed by any HPE documentation. Confirm empirically (e.g. drive load on a single visible GPU and observe which accel[i]_power reacts) before relying on this for analysis.\"\n"
                       "  verified: false\n"
                       "  accel_indices_discovered: [{}]\n"
                       "  gpu_vendors:\n"
                       "{}"
                       "\n",
                       fmt::join(accel_indices, ", "),
                       vendor_blocks);
}
#else
std::string system_hardware_sampler::device_correlation_hints_as_yaml_string() const {
    return "";
}
#endif

void system_hardware_sampler::create_local_samplers(std::chrono::milliseconds sampling_interval, sample_category category) {
#if defined(HWS_FOR_CPUS_ENABLED)
    {
        samplers_.push_back(std::make_unique<cpu_hardware_sampler>(sampling_interval, category));
    }
#endif
#if defined(HWS_FOR_CRAY_PM_COUNTERS_ENABLED)
    if (detail::pm_counters_available()) {
        samplers_.push_back(std::make_unique<cray_pm_counters_hardware_sampler>(sampling_interval, category));
    }
#endif
#if defined(HWS_FOR_NVIDIA_GPUS_ENABLED)
    {
        int device_count{};
        HWS_CUDA_ERROR_CHECK(cudaGetDeviceCount(&device_count));
        for (int device = 0; device < device_count; ++device) {
            samplers_.push_back(std::make_unique<gpu_nvidia_hardware_sampler>(static_cast<std::size_t>(device), sampling_interval, category));
        }
    }
#endif
#if defined(HWS_FOR_AMD_GPUS_ENABLED)
    {
        int device_count{};
        HWS_HIP_ERROR_CHECK(hipGetDeviceCount(&device_count));
        for (int device = 0; device < device_count; ++device) {
            samplers_.push_back(std::make_unique<gpu_amd_hardware_sampler>(static_cast<std::size_t>(device), sampling_interval, category));
        }
    }
#endif
#if defined(HWS_FOR_INTEL_GPUS_ENABLED)
    {
        // init level zero driver
        HWS_LEVEL_ZERO_ERROR_CHECK(zeInit(ZE_INIT_FLAG_GPU_ONLY))

        // discover the number of drivers
        std::uint32_t driver_count{ 0 };
        HWS_LEVEL_ZERO_ERROR_CHECK(zeDriverGet(&driver_count, nullptr))

        // check if only the single GPU driver has been found
        if (driver_count > 1) {
            throw std::runtime_error{ fmt::format("Found too many GPU drivers ({})!", driver_count) };
        }

        // get the GPU driver
        ze_driver_handle_t driver{};
        HWS_LEVEL_ZERO_ERROR_CHECK(zeDriverGet(&driver_count, &driver))

        // get all GPUs for the current driver
        std::uint32_t device_count{ 0 };
        HWS_LEVEL_ZERO_ERROR_CHECK(zeDeviceGet(driver, &device_count, nullptr))
        for (std::uint32_t device = 0; device < device_count; ++device) {
            samplers_.push_back(std::make_unique<gpu_intel_hardware_sampler>(static_cast<std::size_t>(device), sampling_interval, category));
        }
    }
#endif
}

}  // namespace hws
