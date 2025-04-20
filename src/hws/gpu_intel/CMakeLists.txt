## Authors: Marcel Breyer
## Copyright (C): 2024-today All Rights Reserved
## License: This file is released under the MIT license.
##          See the LICENSE.md file in the project root for full license information.
########################################################################################################################

# set the CMAKE_MODULE_PATH to the cmake directory
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# try finding Level Zero
find_package(level_zero QUIET)

# check if NVML could be found
if (NOT level_zero_FOUND)
    if (HWS_ENABLE_GPU_INTEL_SAMPLING MATCHES "ON")
        message(SEND_ERROR "Cannot find Level Zero but Intel GPU sampling was explicitly requested!")
    else ()
        message(STATUS "Cannot find Level Zero. Hardware sampling for Intel GPUs disabled.")
    endif ()
    return()
endif ()
message(STATUS "Enable sampling of Intel GPU information using Level Zero.")

# link against necessary libraries
target_link_libraries(${HWS_LIBRARY_NAME} PRIVATE level_zero)

# add source file to source file list
target_sources(${HWS_LIBRARY_NAME} PRIVATE
        $<BUILD_INTERFACE:
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware_sampler.cpp;
        ${CMAKE_CURRENT_SOURCE_DIR}/level_zero_samples.cpp;
        ${CMAKE_CURRENT_SOURCE_DIR}/utility.cpp
        >)

# add compile definition
target_compile_definitions(${HWS_LIBRARY_NAME} PUBLIC HWS_FOR_INTEL_GPUS_ENABLED)
