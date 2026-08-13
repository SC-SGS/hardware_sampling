/**
 * @author Alexander Van Craen
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Tests for the pure enum/bitmask <-> string conversion helpers in the gpu_nvidia backend.
 * @details No NVIDIA GPU or NVML driver required - only the NVML headers, for the bitmask constants.
 */

#include "hws/gpu_nvidia/utility.hpp"  // hws::detail::throttle_event_reason_to_string, CUDA_VERSION

#include "doctest/doctest.h"

#if CUDA_VERSION >= 12000

TEST_CASE("throttle_event_reason_to_string") {
    SUBCASE("zero mask") {
        CHECK_EQ(hws::detail::throttle_event_reason_to_string(0ull), "None");
    }
    SUBCASE("a single reason") {
        CHECK_EQ(hws::detail::throttle_event_reason_to_string(nvmlClocksEventReasonGpuIdle), "GpuIdle");
        CHECK_EQ(hws::detail::throttle_event_reason_to_string(nvmlClocksThrottleReasonHwSlowdown), "HwSlowdown");
    }
    SUBCASE("combined reasons are joined with '|', in the function's fixed check order (not bit-value order)") {
        // DisplayClockSetting (checked 2nd, bit 0x100) | GpuIdle (checked 3rd, bit 0x1): bit-ascending order would
        // print GpuIdle first (0x1 < 0x100) - only the function's actual check order prints DisplayClockSetting
        // first, so this pair actually discriminates the two hypotheses (verified against the real nvml.h values)
        const unsigned long long mask = nvmlClocksEventReasonDisplayClockSetting | nvmlClocksEventReasonGpuIdle;
        CHECK_EQ(hws::detail::throttle_event_reason_to_string(mask), "DisplayClockSetting|GpuIdle");

        // HwPowerBrakeSlowdown (checked 7th, bit 0x80) | HwSlowdown (checked 8th, bit 0x8): same discriminating
        // property as above, using the two throttle (not event) reasons
        const unsigned long long second_pair = nvmlClocksThrottleReasonHwPowerBrakeSlowdown | nvmlClocksThrottleReasonHwSlowdown;
        CHECK_EQ(hws::detail::throttle_event_reason_to_string(second_pair), "HwPowerBrakeSlowdown|HwSlowdown");
    }
}

#else

TEST_CASE("throttle_event_reason_to_string is not compiled for CUDA < 12.0") {
    MESSAGE("Skipped: this CUDA toolkit is older than 12.0; hws::detail::throttle_event_reason_to_string doesn't exist in this build.");
}

#endif
