## Authors: Alexander Van Craen
## Copyright (C): 2024-today All Rights Reserved
## License: This file is released under the MIT license.
##          See the LICENSE.md file in the project root for full license information.
########################################################################################################################

# pm_counters is a Linux-only (Cray/HPE) sysfs interface, not a linkable library -> nothing to find_package/find_program.
# Availability of /sys/cray/pm_counters is checked at runtime (build host and target host may differ, e.g. cross-compiling
# on a login node for execution on a compute node), so this backend is always compiled in on Linux.
if (NOT UNIX)
    if (HWS_ENABLE_CRAY_PM_COUNTERS_SAMPLING MATCHES "ON")
        message(SEND_ERROR "Cray pm_counters sampling was explicitly requested but is only available on Linux!")
    else ()
        message(STATUS "Cray pm_counters sampling is only available on Linux. Hardware sampling via pm_counters disabled.")
    endif ()
    return()
endif ()
message(STATUS "Enable sampling of node power/energy via /sys/cray/pm_counters (availability checked at runtime).")

# add source file to source file list
target_sources(${HWS_LIBRARY_NAME} PRIVATE
        $<BUILD_INTERFACE:
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware_sampler.cpp;
        ${CMAKE_CURRENT_SOURCE_DIR}/pm_counters_samples.cpp;
        ${CMAKE_CURRENT_SOURCE_DIR}/utility.cpp
        >)

# add compile definition
target_compile_definitions(${HWS_LIBRARY_NAME} PUBLIC HWS_FOR_CRAY_PM_COUNTERS_ENABLED)
