/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hws/gpu_nvidia/utility.hpp"

#include "fmt/format.h"  // fmt::format
#include "fmt/ranges.h"  // fmt::join
#include "nvml.h"        // NVML runtime functions

#include <string>  // std::string
#include <vector>  // std::vector

namespace hws::detail {

std::string throttle_event_reason_to_string(const unsigned long long clocks_event_reasons) {
    if (clocks_event_reasons == 0ull) {
        return "None";
    } else {
        std::vector<std::string> reasons{};
        if ((clocks_event_reasons & nvmlClocksEventReasonApplicationsClocksSetting) != 0ull) {
            reasons.emplace_back("ApplicationsClocksSetting");
        }
        if ((clocks_event_reasons & nvmlClocksEventReasonDisplayClockSetting) != 0ull) {
            reasons.emplace_back("DisplayClockSetting");
        }
        if ((clocks_event_reasons & nvmlClocksEventReasonGpuIdle) != 0ull) {
            reasons.emplace_back("GpuIdle");
        }
        if ((clocks_event_reasons & nvmlClocksEventReasonSwPowerCap) != 0ull) {
            reasons.emplace_back("SwPowerCap");
        }
        if ((clocks_event_reasons & nvmlClocksEventReasonSwThermalSlowdown) != 0ull) {
            reasons.emplace_back("SwThermalSlowdown");
        }
        if ((clocks_event_reasons & nvmlClocksEventReasonSyncBoost) != 0ull) {
            reasons.emplace_back("SyncBoost");
        }
        if ((clocks_event_reasons & nvmlClocksThrottleReasonHwPowerBrakeSlowdown) != 0ull) {
            reasons.emplace_back("HwPowerBrakeSlowdown");
        }
        if ((clocks_event_reasons & nvmlClocksThrottleReasonHwSlowdown) != 0ull) {
            reasons.emplace_back("HwSlowdown");
        }
        if ((clocks_event_reasons & nvmlClocksThrottleReasonHwThermalSlowdown) != 0ull) {
            reasons.emplace_back("HwThermalSlowdown");
        }
        return fmt::format("{}", fmt::join(reasons, "|"));
    }
}

}  // namespace hws::detail
