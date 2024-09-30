/**
 * @file
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Core header containing all other necessary other headers.
 */

#ifndef HARDWARE_SAMPLING_CORE_HPP_
#define HARDWARE_SAMPLING_CORE_HPP_
#pragma once

#include "hardware_sampling/event.hpp"
#include "hardware_sampling/hardware_sampler.hpp"
#include "hardware_sampling/sample_category.hpp"
#include "hardware_sampling/system_hardware_sampler.hpp"
#include "hardware_sampling/version.hpp"

#if defined(HWS_FOR_CPUS_ENABLED)
    #include "hardware_sampling/cpu/cpu_samples.hpp"
    #include "hardware_sampling/cpu/hardware_sampler.hpp"
#endif

#if defined(HWS_FOR_NVIDIA_GPUS_ENABLED)
    #include "hardware_sampling/gpu_nvidia//nvml_samples.hpp"
    #include "hardware_sampling/gpu_nvidia/hardware_sampler.hpp"
#endif

#if defined(HWS_FOR_AMD_GPUS_ENABLED)
    #include "hardware_sampling/gpu_amd/hardware_sampler.hpp"
    #include "hardware_sampling/gpu_amd/rocm_smi_samples.hpp"
#endif

#if defined(HWS_FOR_INTEL_GPUS_ENABLED)
    #include "hardware_sampling/gpu_intel/hardware_sampler.hpp"
    #include "hardware_sampling/gpu_intel/level_zero_samples.hpp"
#endif

#endif  // HARDWARE_SAMPLING_CORE_HPP_
