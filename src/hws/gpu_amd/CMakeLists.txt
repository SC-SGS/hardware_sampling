## Authors: Marcel Breyer
## Copyright (C): 2024-today All Rights Reserved
## License: This file is released under the MIT license.
##          See the LICENSE.md file in the project root for full license information.
########################################################################################################################

# try finding ROCm SMI
find_package(rocm_smi QUIET)

# check if ROCm SMI could be found
if (NOT rocm_smi_FOUND)
    if (HWS_ENABLE_GPU_AMD_SAMPLING MATCHES "ON")
        message(SEND_ERROR "Cannot find ROCm SMI but AMD GPU sampling was explicitly requested!")
    else ()
        message(STATUS "Cannot find ROCm SMI. Hardware sampling for AMD GPUs disabled.")
    endif ()
    return()
endif ()
message(STATUS "Enable sampling of AMD GPU information using ROCm SMI.")

# must also find HIP
find_package(HIP REQUIRED)

# link against necessary libraries
target_link_libraries(${HWS_LIBRARY_NAME} PRIVATE rocm_smi64 hip::host)
target_include_directories(${HWS_LIBRARY_NAME} PRIVATE ${ROCM_SMI_INCLUDE_DIR})

# add source file to source file list
target_sources(${HWS_LIBRARY_NAME} PRIVATE
        $<BUILD_INTERFACE:
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware_sampler.cpp;
        ${CMAKE_CURRENT_SOURCE_DIR}/rocm_smi_samples.cpp;
        ${CMAKE_CURRENT_SOURCE_DIR}/utility.cpp
        >)

# add compile definition
target_compile_definitions(${HWS_LIBRARY_NAME} PUBLIC HWS_FOR_AMD_GPUS_ENABLED)
