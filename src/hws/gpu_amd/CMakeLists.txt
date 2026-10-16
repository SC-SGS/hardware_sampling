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
message(STATUS "Enable sampling of AMD GPU information using ROCm SMI (${rocm_smi_VERSION}).")

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

# select the source of the total energy consumption: ROCm SMI's energy counter can't be queried in parallel by multiple
# AMD GPU samplers, so AUTO integrates the energy from the power usage if more than one AMD GPU sampler exists (or always
# in MPI builds, since multiple MPI ranks on the same node may each own AMD GPU samplers)
set(HWS_AMD_GPU_ENERGY_SOURCE AUTO CACHE STRING "Source of the AMD GPU total energy consumption: AUTO (integrate from power if more than one AMD GPU sampler exists or MPI support is enabled, otherwise use the ROCm SMI energy counter), COUNTER, or INTEGRATED.")
set_property(CACHE HWS_AMD_GPU_ENERGY_SOURCE PROPERTY STRINGS AUTO COUNTER INTEGRATED)
if (HWS_AMD_GPU_ENERGY_SOURCE MATCHES "^COUNTER$")
    target_compile_definitions(${HWS_LIBRARY_NAME} PRIVATE HWS_AMD_GPU_ENERGY_SOURCE_COUNTER)
elseif (HWS_AMD_GPU_ENERGY_SOURCE MATCHES "^INTEGRATED$")
    target_compile_definitions(${HWS_LIBRARY_NAME} PRIVATE HWS_AMD_GPU_ENERGY_SOURCE_INTEGRATED)
elseif (NOT HWS_AMD_GPU_ENERGY_SOURCE MATCHES "^AUTO$")
    message(SEND_ERROR "HWS_AMD_GPU_ENERGY_SOURCE must be AUTO, COUNTER, or INTEGRATED, but is \"${HWS_AMD_GPU_ENERGY_SOURCE}\"!")
endif ()
message(STATUS "AMD GPU energy source: ${HWS_AMD_GPU_ENERGY_SOURCE}")
