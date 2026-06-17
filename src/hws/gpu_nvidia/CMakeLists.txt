## Authors: Marcel Breyer
## Copyright (C): 2024-today All Rights Reserved
## License: This file is released under the MIT license.
##          See the LICENSE.md file in the project root for full license information.
########################################################################################################################

# try finding NVML
find_package(CUDAToolkit QUIET)

# check if NVML could be found
if (NOT CUDAToolkit_FOUND)
    if (HWS_ENABLE_GPU_NVIDIA_SAMPLING MATCHES "ON")
        message(SEND_ERROR "Cannot find NVML but NVIDIA GPU sampling was explicitly requested!")
    else ()
        message(STATUS "Cannot find NVML. Hardware sampling for NVIDIA GPUs disabled.")
    endif ()
    return()
endif ()
message(STATUS "Enable sampling of NVIDIA GPU information using NVML (${CUDAToolkit_VERSION}).")

# link against necessary libraries
target_link_libraries(${HWS_LIBRARY_NAME} PRIVATE CUDA::nvml CUDA::cudart)

# add source file to source file list
target_sources(${HWS_LIBRARY_NAME} PRIVATE
        $<BUILD_INTERFACE:
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware_sampler.cpp;
        ${CMAKE_CURRENT_SOURCE_DIR}/nvml_samples.cpp;
        ${CMAKE_CURRENT_SOURCE_DIR}/utility.cpp
        >)

# add compile definition
target_compile_definitions(${HWS_LIBRARY_NAME} PUBLIC HWS_FOR_NVIDIA_GPUS_ENABLED)
