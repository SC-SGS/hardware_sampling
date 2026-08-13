/**
 * @author Alexander Van Craen
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Tests for the pure enum <-> string conversion helper in the gpu_amd backend.
 * @details No AMD GPU or ROCm SMI driver required - only the ROCm SMI headers, for the enum definition.
 */

#include "hws/gpu_amd/utility.hpp"  // hws::detail::performance_level_to_string

#include "doctest/doctest.h"

TEST_CASE("performance_level_to_string") {
    CHECK_EQ(hws::detail::performance_level_to_string(RSMI_DEV_PERF_LEVEL_AUTO), "auto");
    CHECK_EQ(hws::detail::performance_level_to_string(RSMI_DEV_PERF_LEVEL_LOW), "low");
    CHECK_EQ(hws::detail::performance_level_to_string(RSMI_DEV_PERF_LEVEL_HIGH), "high");
    CHECK_EQ(hws::detail::performance_level_to_string(RSMI_DEV_PERF_LEVEL_MANUAL), "manual");
    CHECK_EQ(hws::detail::performance_level_to_string(RSMI_DEV_PERF_LEVEL_STABLE_STD), "stable_std");
    CHECK_EQ(hws::detail::performance_level_to_string(RSMI_DEV_PERF_LEVEL_STABLE_PEAK), "stable_peak");
    CHECK_EQ(hws::detail::performance_level_to_string(RSMI_DEV_PERF_LEVEL_STABLE_MIN_MCLK), "stable_min_mclk");
    CHECK_EQ(hws::detail::performance_level_to_string(RSMI_DEV_PERF_LEVEL_STABLE_MIN_SCLK), "stable_min_sclk");
    CHECK_EQ(hws::detail::performance_level_to_string(RSMI_DEV_PERF_LEVEL_DETERMINISM), "determinism");
    CHECK_EQ(hws::detail::performance_level_to_string(RSMI_DEV_PERF_LEVEL_UNKNOWN), "unknown");

    // a value outside the defined enumerators must hit the `default:` branch, not e.g. throw or read garbage
    CHECK_EQ(hws::detail::performance_level_to_string(static_cast<rsmi_dev_perf_level_t>(9999)), "unknown");
}
