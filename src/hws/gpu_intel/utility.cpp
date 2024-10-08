/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hws/gpu_intel/utility.hpp"

#include "fmt/format.h"          // fmt::format
#include "fmt/ranges.h"          // fmt::join
#include "level_zero/ze_api.h"   // Level Zero runtime functions
#include "level_zero/zes_api.h"  // Level Zero runtime functions

#include <string>       // std::string
#include <string_view>  // std::string_view
#include <vector>       // std::vector

namespace hws::detail {

std::vector<std::string> property_flags_to_vector(const ze_device_property_flags_t flags) {
    std::vector<std::string> string_flags{};

    if ((flags & ze_device_property_flag_t::ZE_DEVICE_PROPERTY_FLAG_INTEGRATED) != 0) {
        string_flags.emplace_back("integrated_gpu");
    }
    if ((flags & ze_device_property_flag_t::ZE_DEVICE_PROPERTY_FLAG_SUBDEVICE) != 0) {
        string_flags.emplace_back("sub-device");
    }
    if ((flags & ze_device_property_flag_t::ZE_DEVICE_PROPERTY_FLAG_ECC) != 0) {
        string_flags.emplace_back("ecc");
    }
    if ((flags & ze_device_property_flag_t::ZE_DEVICE_PROPERTY_FLAG_ONDEMANDPAGING) != 0) {
        string_flags.emplace_back("on-demand_page-faulting");
    }

    return string_flags;
}

std::string throttle_reason_to_string(const zes_freq_throttle_reason_flags_t reasons) {
    if (reasons == 0) {
        return "None";
    } else {
        std::vector<std::string> string_reasons{};
        if ((reasons & zes_freq_throttle_reason_flag_t::ZES_FREQ_THROTTLE_REASON_FLAG_AVE_PWR_CAP) != 0) {
            string_reasons.emplace_back("average_power");
        }
        if ((reasons & zes_freq_throttle_reason_flag_t::ZES_FREQ_THROTTLE_REASON_FLAG_BURST_PWR_CAP) != 0) {
            string_reasons.emplace_back("burst_power");
        }
        if ((reasons & zes_freq_throttle_reason_flag_t::ZES_FREQ_THROTTLE_REASON_FLAG_CURRENT_LIMIT) != 0) {
            string_reasons.emplace_back("current_limit");
        }
        if ((reasons & zes_freq_throttle_reason_flag_t::ZES_FREQ_THROTTLE_REASON_FLAG_THERMAL_LIMIT) != 0) {
            string_reasons.emplace_back("thermal_limit");
        }
        if ((reasons & zes_freq_throttle_reason_flag_t::ZES_FREQ_THROTTLE_REASON_FLAG_PSU_ALERT) != 0) {
            string_reasons.emplace_back("psu_assertion");
        }
        if ((reasons & zes_freq_throttle_reason_flag_t::ZES_FREQ_THROTTLE_REASON_FLAG_SW_RANGE) != 0) {
            string_reasons.emplace_back("software_frequency_range");
        }
        if ((reasons & zes_freq_throttle_reason_flag_t::ZES_FREQ_THROTTLE_REASON_FLAG_HW_RANGE) != 0) {
            string_reasons.emplace_back("hardware_frequency_range");
        }
        return fmt::format("{}", fmt::join(string_reasons, "|"));
    }
}

std::string_view to_result_string(const ze_result_t errc) {
    switch (errc) {
        case ZE_RESULT_SUCCESS:
            return "ZE_RESULT_SUCCESS: success [core]";
        case ZE_RESULT_NOT_READY:
            return "ZE_RESULT_NOT_READY: synchronization primitive not signaled [core]";
        case ZE_RESULT_ERROR_DEVICE_LOST:
            return "ZE_RESULT_ERROR_DEVICE_LOST: device hung, reset, was removed, or driver update occurred [core]";
        case ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY:
            return "ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY: insufficient host memory to satisfy call [core]";
        case ZE_RESULT_ERROR_OUT_OF_DEVICE_MEMORY:
            return "ZE_RESULT_ERROR_OUT_OF_DEVICE_MEMORY: insufficient device memory to satisfy call [core]";
        case ZE_RESULT_ERROR_MODULE_BUILD_FAILURE:
            return "ZE_RESULT_ERROR_MODULE_BUILD_FAILURE: error occurred when building module, see build log for details [core]";
        case ZE_RESULT_ERROR_MODULE_LINK_FAILURE:
            return "ZE_RESULT_ERROR_MODULE_LINK_FAILURE: error occurred when linking modules, see build log for details [core]";
        case ZE_RESULT_ERROR_DEVICE_REQUIRES_RESET:
            return "ZE_RESULT_ERROR_DEVICE_REQUIRES_RESET: device requires a reset [core]";
        case ZE_RESULT_ERROR_DEVICE_IN_LOW_POWER_STATE:
            return "ZE_RESULT_ERROR_DEVICE_IN_LOW_POWER_STATE: device currently in low power state [core]";
        case ZE_RESULT_EXP_ERROR_DEVICE_IS_NOT_VERTEX:
            return "ZE_RESULT_EXP_ERROR_DEVICE_IS_NOT_VERTEX: device is not represented by a fabric vertex [core, experimental]";
        case ZE_RESULT_EXP_ERROR_VERTEX_IS_NOT_DEVICE:
            return "ZE_RESULT_EXP_ERROR_VERTEX_IS_NOT_DEVICE: fabric vertex does not represent a device [core, experimental]";
        case ZE_RESULT_EXP_ERROR_REMOTE_DEVICE:
            return "ZE_RESULT_EXP_ERROR_REMOTE_DEVICE: fabric vertex represents a remote device or subdevice [core, experimental]";
        // case ZE_RESULT_EXP_ERROR_OPERANDS_INCOMPATIBLE:
        //     return "ZE_RESULT_EXP_ERROR_OPERANDS_INCOMPATIBLE: operands of comparison are not compatible [core, experimental]";
        // case ZE_RESULT_EXP_RTAS_BUILD_RETRY:
        //     return "ZE_RESULT_EXP_RTAS_BUILD_RETRY: ray tracing acceleration structure build operation failed due to insufficient resources, retry with a larger acceleration structure buffer allocation [core, experimental]";
        // case ZE_RESULT_EXP_RTAS_BUILD_DEFERRED:
        //     return "ZE_RESULT_EXP_RTAS_BUILD_DEFERRED: ray tracing acceleration structure build operation deferred to parallel operation join [core, experimental]";
        case ZE_RESULT_ERROR_INSUFFICIENT_PERMISSIONS:
            return "ZE_RESULT_ERROR_INSUFFICIENT_PERMISSIONS: access denied due to permission level [sysman]";
        case ZE_RESULT_ERROR_NOT_AVAILABLE:
            return "ZE_RESULT_ERROR_NOT_AVAILABLE: resource already in use and simultaneous access not allowed or resource was removed [sysman]";
        case ZE_RESULT_ERROR_DEPENDENCY_UNAVAILABLE:
            return "ZE_RESULT_ERROR_DEPENDENCY_UNAVAILABLE: external required dependency is unavailable or missing [common]";
        case ZE_RESULT_WARNING_DROPPED_DATA:
            return "ZE_RESULT_WARNING_DROPPED_DATA: data may have been dropped [tools]";
        case ZE_RESULT_ERROR_UNINITIALIZED:
            return "ZE_RESULT_ERROR_UNINITIALIZED: driver is not initialized [validation]";
        case ZE_RESULT_ERROR_UNSUPPORTED_VERSION:
            return "ZE_RESULT_ERROR_UNSUPPORTED_VERSION: generic error code for unsupported versions [validation]";
        case ZE_RESULT_ERROR_UNSUPPORTED_FEATURE:
            return "ZE_RESULT_ERROR_UNSUPPORTED_FEATURE: generic error code for unsupported features [validation]";
        case ZE_RESULT_ERROR_INVALID_ARGUMENT:
            return "ZE_RESULT_ERROR_INVALID_ARGUMENT: generic error code for invalid arguments [validation]";
        case ZE_RESULT_ERROR_INVALID_NULL_HANDLE:
            return "ZE_RESULT_ERROR_INVALID_NULL_HANDLE: handle argument is not valid [validation]";
        case ZE_RESULT_ERROR_HANDLE_OBJECT_IN_USE:
            return "ZE_RESULT_ERROR_HANDLE_OBJECT_IN_USE: object pointed to by handle still in-use by device [validation]";
        case ZE_RESULT_ERROR_INVALID_NULL_POINTER:
            return "ZE_RESULT_ERROR_INVALID_NULL_POINTER: pointer argument may not be nullptr [validation]";
        case ZE_RESULT_ERROR_INVALID_SIZE:
            return "ZE_RESULT_ERROR_INVALID_SIZE: size argument is invalid (e.g., must not be zero) [validation]";
        case ZE_RESULT_ERROR_UNSUPPORTED_SIZE:
            return "ZE_RESULT_ERROR_UNSUPPORTED_SIZE: size argument is not supported by the device (e.g., too large) [validation]";
        case ZE_RESULT_ERROR_UNSUPPORTED_ALIGNMENT:
            return "ZE_RESULT_ERROR_UNSUPPORTED_ALIGNMENT: alignment argument is not supported by the device (e.g., too small) [validation]";
        case ZE_RESULT_ERROR_INVALID_SYNCHRONIZATION_OBJECT:
            return "ZE_RESULT_ERROR_INVALID_SYNCHRONIZATION_OBJECT: synchronization object in invalid state [validation]";
        case ZE_RESULT_ERROR_INVALID_ENUMERATION:
            return "ZE_RESULT_ERROR_INVALID_ENUMERATION: enumerator argument is not valid [validation]";
        case ZE_RESULT_ERROR_UNSUPPORTED_ENUMERATION:
            return "ZE_RESULT_ERROR_UNSUPPORTED_ENUMERATION: enumerator argument is not supported by the device [validation]";
        case ZE_RESULT_ERROR_UNSUPPORTED_IMAGE_FORMAT:
            return "ZE_RESULT_ERROR_UNSUPPORTED_IMAGE_FORMAT: image format is not supported by the device [validation]";
        case ZE_RESULT_ERROR_INVALID_NATIVE_BINARY:
            return "ZE_RESULT_ERROR_INVALID_NATIVE_BINARY: native binary is not supported by the device [validation]";
        case ZE_RESULT_ERROR_INVALID_GLOBAL_NAME:
            return "ZE_RESULT_ERROR_INVALID_GLOBAL_NAME: global variable is not found in the module [validation]";
        case ZE_RESULT_ERROR_INVALID_KERNEL_NAME:
            return "ZE_RESULT_ERROR_INVALID_KERNEL_NAME: kernel name is not found in the module [validation]";
        case ZE_RESULT_ERROR_INVALID_FUNCTION_NAME:
            return "ZE_RESULT_ERROR_INVALID_FUNCTION_NAME: function name is not found in the module [validation]";
        case ZE_RESULT_ERROR_INVALID_GROUP_SIZE_DIMENSION:
            return "ZE_RESULT_ERROR_INVALID_GROUP_SIZE_DIMENSION: group size dimension is not valid for the kernel or device [validation]";
        case ZE_RESULT_ERROR_INVALID_GLOBAL_WIDTH_DIMENSION:
            return "ZE_RESULT_ERROR_INVALID_GLOBAL_WIDTH_DIMENSION: global width dimension is not valid for the kernel or device [validation]";
        case ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_INDEX:
            return "ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_INDEX: kernel argument index is not valid for kernel [validation]";
        case ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_SIZE:
            return "ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_SIZE: kernel argument size does not match kernel [validation]";
        case ZE_RESULT_ERROR_INVALID_KERNEL_ATTRIBUTE_VALUE:
            return "ZE_RESULT_ERROR_INVALID_KERNEL_ATTRIBUTE_VALUE: value of kernel attribute is not valid for the kernel or device [validation]";
        case ZE_RESULT_ERROR_INVALID_MODULE_UNLINKED:
            return "ZE_RESULT_ERROR_INVALID_MODULE_UNLINKED: module with imports needs to be linked before kernels can be created from it [validation]";
        case ZE_RESULT_ERROR_INVALID_COMMAND_LIST_TYPE:
            return "ZE_RESULT_ERROR_INVALID_COMMAND_LIST_TYPE: command list type does not match command queue type [validation]";
        case ZE_RESULT_ERROR_OVERLAPPING_REGIONS:
            return "ZE_RESULT_ERROR_OVERLAPPING_REGIONS: copy operations do not support overlapping regions of memory [validation]";
        case ZE_RESULT_WARNING_ACTION_REQUIRED:
            return "ZE_RESULT_WARNING_ACTION_REQUIRED: an action is required to complete the desired operation [sysman]";
        case ZE_RESULT_ERROR_UNKNOWN:
            return "ZE_RESULT_ERROR_UNKNOWN: unknown or internal error [core]";
        case ZE_RESULT_FORCE_UINT32:
            return "ZE_RESULT_FORCE_UINT32";
        default:
            return "unknown level zero error";
    }
}

std::string memory_module_to_name(const zes_mem_type_t mem_type) {
    switch (mem_type) {
        case ZES_MEM_TYPE_HBM:
            return "hbm";
        case ZES_MEM_TYPE_DDR:
            return "ddr";
        case ZES_MEM_TYPE_DDR3:
            return "ddr3";
        case ZES_MEM_TYPE_DDR4:
            return "ddr4";
        case ZES_MEM_TYPE_DDR5:
            return "ddr5";
        case ZES_MEM_TYPE_LPDDR:
            return "lpddr";
        case ZES_MEM_TYPE_LPDDR3:
            return "lpddr3";
        case ZES_MEM_TYPE_LPDDR4:
            return "lpddr4";
        case ZES_MEM_TYPE_LPDDR5:
            return "lpddr5";
        case ZES_MEM_TYPE_SRAM:
            return "sram";
        case ZES_MEM_TYPE_L1:
            return "l1";
        case ZES_MEM_TYPE_L3:
            return "l3";
        case ZES_MEM_TYPE_GRF:
            return "grf";
        case ZES_MEM_TYPE_SLM:
            return "slm";
        case ZES_MEM_TYPE_GDDR4:
            return "gddr4";
        case ZES_MEM_TYPE_GDDR5:
            return "gddr5";
        case ZES_MEM_TYPE_GDDR5X:
            return "gddr5x";
        case ZES_MEM_TYPE_GDDR6:
            return "gddr6";
        case ZES_MEM_TYPE_GDDR6X:
            return "gddr6x";
        case ZES_MEM_TYPE_GDDR7:
            return "gddr7";
        default: return "";
    }
}

std::string memory_location_to_name(const zes_mem_loc_t mem_loc) {
    switch (mem_loc) {
        case ZES_MEM_LOC_SYSTEM:
            return "system";
        case ZES_MEM_LOC_DEVICE:
            return "device";
        default:
            return "";
    }
}

}  // namespace hws::detail
