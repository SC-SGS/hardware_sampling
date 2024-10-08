/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hws/gpu_amd/utility.hpp"

#include "rocm_smi/rocm_smi.h"  // ROCm SMI runtime functions

#include <string>  // std::string

namespace hws::detail {

std::string performance_level_to_string(const rsmi_dev_perf_level_t perf_level) {
    switch (perf_level) {
        case RSMI_DEV_PERF_LEVEL_AUTO:
            return "auto";
        case RSMI_DEV_PERF_LEVEL_LOW:
            return "low";
        case RSMI_DEV_PERF_LEVEL_HIGH:
            return "high";
        case RSMI_DEV_PERF_LEVEL_MANUAL:
            return "manual";
        case RSMI_DEV_PERF_LEVEL_STABLE_STD:
            return "stable_std";
        case RSMI_DEV_PERF_LEVEL_STABLE_PEAK:
            return "stable_peak";
        case RSMI_DEV_PERF_LEVEL_STABLE_MIN_MCLK:
            return "stable_min_mclk";
        case RSMI_DEV_PERF_LEVEL_STABLE_MIN_SCLK:
            return "stable_min_sclk";
        case RSMI_DEV_PERF_LEVEL_DETERMINISM:
            return "determinism";
        case RSMI_DEV_PERF_LEVEL_UNKNOWN:
        default:
            return "unknown";
    }
}

}  // namespace hws::detail
