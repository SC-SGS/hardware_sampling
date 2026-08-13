/**
 * @author Alexander Van Craen
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Tests for the pure enum/bitmask <-> string conversion helpers in the gpu_intel backend.
 * @details No Intel GPU or Level Zero driver required - only the Level Zero headers, for the enum/bitmask definitions.
 */

#include "hws/gpu_intel/utility.hpp"  // hws::detail::{to_result_string, memory_module_to_name, memory_location_to_name, property_flags_to_vector, throttle_reason_to_string}

#include "doctest/doctest.h"

#include <string>  // std::string
#include <vector>  // std::vector

TEST_CASE("to_result_string") {
    CHECK_EQ(hws::detail::to_result_string(ZE_RESULT_SUCCESS), "ZE_RESULT_SUCCESS: success [core]");
    CHECK_EQ(hws::detail::to_result_string(ZE_RESULT_ERROR_DEVICE_LOST), "ZE_RESULT_ERROR_DEVICE_LOST: device hung, reset, was removed, or driver update occurred [core]");
    CHECK_EQ(hws::detail::to_result_string(ZE_RESULT_ERROR_UNKNOWN), "ZE_RESULT_ERROR_UNKNOWN: unknown or internal error [core]");
    // a value with no matching case must hit the `default:` branch; real ze_result_t values are 0, 1, then jump
    // straight to 0x70000001+ (verified against ze_api.h), so 2 is guaranteed not to collide with any enumerator
    CHECK_EQ(hws::detail::to_result_string(static_cast<ze_result_t>(2)), "unknown level zero error");
}

TEST_CASE("memory_module_to_name") {
    CHECK_EQ(hws::detail::memory_module_to_name(ZES_MEM_TYPE_HBM), "hbm");
    CHECK_EQ(hws::detail::memory_module_to_name(ZES_MEM_TYPE_DDR4), "ddr4");
    CHECK_EQ(hws::detail::memory_module_to_name(ZES_MEM_TYPE_GDDR6), "gddr6");
    // a value with no matching case falls back to an empty string, not e.g. "unknown"
    CHECK_EQ(hws::detail::memory_module_to_name(static_cast<zes_mem_type_t>(9999)), "");
}

TEST_CASE("memory_location_to_name") {
    CHECK_EQ(hws::detail::memory_location_to_name(ZES_MEM_LOC_SYSTEM), "system");
    CHECK_EQ(hws::detail::memory_location_to_name(ZES_MEM_LOC_DEVICE), "device");
    CHECK_EQ(hws::detail::memory_location_to_name(static_cast<zes_mem_loc_t>(9999)), "");
}

TEST_CASE("property_flags_to_vector") {
    SUBCASE("zero flags") {
        CHECK(hws::detail::property_flags_to_vector(static_cast<ze_device_property_flags_t>(0)).empty());
    }
    SUBCASE("a single flag") {
        const auto flags = static_cast<ze_device_property_flags_t>(ze_device_property_flag_t::ZE_DEVICE_PROPERTY_FLAG_INTEGRATED);
        CHECK_EQ(hws::detail::property_flags_to_vector(flags), std::vector<std::string>{ "integrated_gpu" });
    }
    SUBCASE("combined flags are listed in the function's fixed check order") {
        const auto flags = static_cast<ze_device_property_flags_t>(
                ze_device_property_flag_t::ZE_DEVICE_PROPERTY_FLAG_ECC | ze_device_property_flag_t::ZE_DEVICE_PROPERTY_FLAG_ONDEMANDPAGING);
        CHECK_EQ(hws::detail::property_flags_to_vector(flags), std::vector<std::string>{ "ecc", "on-demand_page-faulting" });
    }
}

TEST_CASE("throttle_reason_to_string") {
    SUBCASE("zero mask") {
        CHECK_EQ(hws::detail::throttle_reason_to_string(static_cast<zes_freq_throttle_reason_flags_t>(0)), "None");
    }
    SUBCASE("a single reason") {
        const auto reasons = static_cast<zes_freq_throttle_reason_flags_t>(zes_freq_throttle_reason_flag_t::ZES_FREQ_THROTTLE_REASON_FLAG_AVE_PWR_CAP);
        CHECK_EQ(hws::detail::throttle_reason_to_string(reasons), "average_power");
    }
    SUBCASE("combined reasons are joined with '|', in the function's fixed check order") {
        const auto reasons = static_cast<zes_freq_throttle_reason_flags_t>(
                zes_freq_throttle_reason_flag_t::ZES_FREQ_THROTTLE_REASON_FLAG_CURRENT_LIMIT | zes_freq_throttle_reason_flag_t::ZES_FREQ_THROTTLE_REASON_FLAG_PSU_ALERT);
        CHECK_EQ(hws::detail::throttle_reason_to_string(reasons), "current_limit|psu_assertion");
    }
}
