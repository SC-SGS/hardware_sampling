/**
 * @file
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Core header containing all other necessary other headers.
 */

#ifndef HWS_CORE_HPP_
#define HWS_CORE_HPP_
#pragma once

#include "hws/event.hpp"
#include "hws/hardware_sampler.hpp"
#include "hws/sample_category.hpp"
#include "hws/system_hardware_sampler.hpp"
#include "hws/version.hpp"

#if defined(HWS_FOR_CPUS_ENABLED)
    #include "hws/cpu/cpu_samples.hpp"
    #include "hws/cpu/hardware_sampler.hpp"
#endif

#if defined(HWS_FOR_NVIDIA_GPUS_ENABLED)
    #include "hws/gpu_nvidia//nvml_samples.hpp"
    #include "hws/gpu_nvidia/hardware_sampler.hpp"
#endif

#if defined(HWS_FOR_AMD_GPUS_ENABLED)
    #include "hws/gpu_amd/hardware_sampler.hpp"
    #include "hws/gpu_amd/rocm_smi_samples.hpp"
#endif

#if defined(HWS_FOR_INTEL_GPUS_ENABLED)
    #include "hws/gpu_intel/hardware_sampler.hpp"
    #include "hws/gpu_intel/level_zero_samples.hpp"
#endif

#endif  // HWS_CORE_HPP_
