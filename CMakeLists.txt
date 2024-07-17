## Authors: Marcel Breyer
## Copyright (C): 2024-today All Rights Reserved
## License: This file is released under the MIT license.
##          See the LICENSE.md file in the project root for full license information.
########################################################################################################################

cmake_minimum_required(VERSION 3.22)

project("HWS - Hardware Sampling for GPUs and CPUs"
        VERSION 1.0.0
        LANGUAGES CXX
        DESCRIPTION "Hardware sampling (e.g., clock frequencies, memory consumption, temperatures, or energy draw) for CPUs, and GPUS.")

# explicitly set library source files
set(HWS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/hardware_sampling/events.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/hardware_sampling/hardware_sampler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/hardware_sampling/utility.cpp
)

# create hardware sampling library
set(HWS_LIBRARY_NAME hws)
add_library(${HWS_LIBRARY_NAME} SHARED ${HWS_SOURCES})

# use C++20
target_compile_features(${HWS_LIBRARY_NAME} PUBLIC cxx_std_20)

# add target include directory
target_include_directories(${HWS_LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# additional library compile options
target_compile_options(${HWS_LIBRARY_NAME} PUBLIC
        $<$<COMPILE_LANGUAGE:CXX>:$<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wdouble-promotion -fno-common -Wshadow -Wcast-qual
        -Wnull-dereference -Wnon-virtual-dtor -Wextra-semi -Wunreachable-code -Wuninitialized -Wno-ctor-dtor-privacy>
        $<$<COMPILE_LANG_AND_ID:CXX,GNU>:-Wsuggest-override -Wstrict-null-sentinel -Wlogical-op -Wduplicated-branches -Wimplicit-fallthrough=5>
        $<$<COMPILE_LANG_AND_ID:CXX,Clang>:-Wmost>
        $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W4 /bigobj /wd4459 /Zc:lambda>>
)

## add option to enable/disable error checking in the hardware sampling functions
option(HWS_ENABLE_ERROR_CHECKS "Enable error checking for the hardware sampling functions. May be problematic with smaller sample intervals." OFF)
if (HWS_ENABLE_ERROR_CHECKS)
    message(STATUS "Enable error checks for the hardware sampling functions.")
    target_compile_definitions(${HWS_LIBRARY_NAME} PRIVATE HWS_ERROR_CHECKS_ENABLED)
endif ()

# specify the sampling interval in milliseconds
set(HWS_SAMPLING_INTERVAL "100" CACHE STRING "The interval in milliseconds in which the hardware information (like clock frequency or power draw) are queried.")
if (NOT ${HWS_SAMPLING_INTERVAL} MATCHES "^[0-9]+$" OR ${HWS_SAMPLING_INTERVAL} LESS_EQUAL 0)
    message(FATAL_ERROR "The HWS_SAMPLING_INTERVAL must be a natural number greater 0, but is \"${HWS_SAMPLING_INTERVAL}\"!")
endif ()
message(STATUS "Setting the hardware sampler interval to ${HWS_SAMPLING_INTERVAL}ms.")
target_compile_definitions(${HWS_LIBRARY_NAME} PUBLIC HWS_SAMPLING_INTERVAL=${HWS_SAMPLING_INTERVAL}ms)


####################################################################################################################
##                                                CPU measurements                                                ##
####################################################################################################################
option(HWS_FOR_CPUS "Enable hardware sampling for the CPU" ON)

if (HWS_FOR_CPUS)
    ## check whether lscpu could be found -> used for the CPU targets as well as for ALL host measurements
    ## -> checked even if no CPU targets where provided
    ## LINUX only
    find_program(HWS_LSCPU_FOUND lscpu)
    if (HWS_LSCPU_FOUND)
        message(STATUS "Enable sampling of CPU information using lscpu.")
        target_compile_definitions(${HWS_LIBRARY_NAME} PUBLIC HWS_VIA_LSCPU_ENABLED)
    endif ()

    ## check whether free could be found -> used for the CPU targets as well as for ALL host measurements
    ## -> checked even if no CPU targets where provided
    ## LINUX only
    find_program(HWS_FREE_FOUND free)
    if (HWS_FREE_FOUND)
        message(STATUS "Enable sampling of CPU information using free.")
        target_compile_definitions(${HWS_LIBRARY_NAME} PUBLIC HWS_VIA_FREE_ENABLED)
    endif ()

    ## check whether turbostat could be found -> used for the CPU targets as well as for ALL host measurements
    ## -> checked even if no CPU targets where provided
    find_program(HWS_TURBOSTAT_FOUND turbostat)
    if (HWS_TURBOSTAT_FOUND)
        ## check if the turbostat command works as intended
        execute_process(COMMAND turbostat -n 1 -S -q
                RESULT_VARIABLE HWS_TURBOSTAT_WITHOUT_ROOT
                OUTPUT_QUIET
                ERROR_QUIET)
        if (NOT HWS_TURBOSTAT_WITHOUT_ROOT EQUAL 0)
            message(STATUS "Can't use turbostat without root permissions! Try using with sudo...")
            execute_process(COMMAND sudo -n turbostat -n 1 -i 0.001 -S -q
                    RESULT_VARIABLE HWS_TURBOSTAT_ROOT_PASSWORD_REQUIRED
                    OUTPUT_QUIET
                    ERROR_QUIET)
            if (NOT HWS_TURBOSTAT_ROOT_PASSWORD_REQUIRED EQUAL 0)
                message(STATUS "Can't use turbostat with root if a sudo password is required! Please add turbostat to the sudoer group.")
                message(STATUS "Disabling turbostat support!")
            else ()
                execute_process(COMMAND sudo turbostat -n 1 -i 0.001 -S -q
                        RESULT_VARIABLE HWS_TURBOSTAT_WITH_ROOT
                        OUTPUT_QUIET
                        ERROR_QUIET)
                if (NOT HWS_TURBOSTAT_WITH_ROOT EQUAL 0)
                    message(STATUS "Can't use turbostat with root even if no sudo password is required!")
                    message(STATUS "Disabling turbostat support!")
                else ()
                    message(STATUS "Enable sampling of CPU information using turbostat.")

                    set(HWS_TURBOSTAT_EXECUTION_TYPE "root")
                    # add compile definitions
                    target_compile_definitions(${HWS_LIBRARY_NAME} PUBLIC HWS_VIA_TURBOSTAT_ENABLED)
                    target_compile_definitions(${HWS_LIBRARY_NAME} PUBLIC HWS_VIA_TURBOSTAT_ROOT)
                endif ()
            endif ()
        else ()
            set(HWS_TURBOSTAT_EXECUTION_TYPE "without_root")
            # add compile definitions
            target_compile_definitions(${HWS_LIBRARY_NAME} PUBLIC HWS_VIA_TURBOSTAT_ENABLED)
        endif ()
    endif ()

    ## check if the CPU hardware tracker can be used
    if (HWS_LSCPU_FOUND OR HWS_FREE_FOUND OR HWS_TURBOSTAT_EXECUTION_TYPE)
        ## try finding subprocess.h
        set(HWS_subprocess_VERSION b6e1611d430e3019c423d2af26bb162e7ed5c3ae)
        find_package(subprocess QUIET)
        if (subprocess_FOUND)
            message(STATUS "Found package subprocess.h.")
            target_include_directories(${HWS_LIBRARY_NAME} PRIVATE ${subprocess_INCLUDE_DIR})
        else ()
            include(FetchContent)
            message(STATUS "Couldn't find package subprocess.h. Building version ${HWS_subprocess_VERSION} from source.")
            # fetch subprocess library subprocess.h
            FetchContent_Declare(subprocess
                    GIT_REPOSITORY https://github.com/sheredom/subprocess.h.git
                    GIT_TAG ${HWS_subprocess_VERSION}
                    QUIET
                    CONFIGURE_COMMAND ""
                    BUILD_COMMAND ""
            )
            FetchContent_MakeAvailable(subprocess)
            target_include_directories(${HWS_LIBRARY_NAME} PRIVATE $<BUILD_INTERFACE:${subprocess_SOURCE_DIR}>)
        endif ()

        # add source file to source file list
        target_sources(${HWS_LIBRARY_NAME} PRIVATE
                $<BUILD_INTERFACE:
                ${CMAKE_CURRENT_SOURCE_DIR}/src/hardware_sampling/cpu/hardware_sampler.cpp;
                ${CMAKE_CURRENT_SOURCE_DIR}/src/hardware_sampling/cpu/cpu_samples.cpp;
                ${CMAKE_CURRENT_SOURCE_DIR}/src/hardware_sampling/cpu/utility.cpp;
                >)

        # add compile definitions
        target_compile_definitions(${HWS_LIBRARY_NAME} PUBLIC HWS_FOR_CPUS_ENABLED)
    else ()
        message(FATAL_ERROR "Hardware sampling for CPUs requested, but none of the required utilities, turbostat, lscpu, or free, can be used!")
    endif ()
endif ()


####################################################################################################################
##                                          NVIDIA GPU sampling via NVML                                          ##
####################################################################################################################
option(HWS_FOR_NVIDIA_GPUS "Enable hardware sampling for NVIDIA GPUs" OFF)

if (HWS_FOR_NVIDIA_GPUS)
    # find libraries necessary for NVML and link against them
    find_package(CUDAToolkit REQUIRED)
    target_link_libraries(${HWS_LIBRARY_NAME} PRIVATE CUDA::nvml)

    message(STATUS "Enable sampling of NVIDIA GPU information using NVML.")

    # add source file to source file list
    target_sources(${HWS_LIBRARY_NAME} PRIVATE
            $<BUILD_INTERFACE:
            ${CMAKE_CURRENT_SOURCE_DIR}/src/hardware_sampling/gpu_nvidia/hardware_sampler.cpp;
            ${CMAKE_CURRENT_SOURCE_DIR}/src/hardware_sampling/gpu_nvidia/nvml_samples.cpp
            >)

    # add compile definition
    target_compile_definitions(${HWS_LIBRARY_NAME} PUBLIC HWS_FOR_NVIDIA_GPUS_ENABLED)
endif ()


####################################################################################################################
##                                        AMD GPU sampling via ROCm SMI lib                                       ##
####################################################################################################################
option(HWS_FOR_AMD_GPUS "Enable hardware sampling for AMD GPUs" OFF)

if (HWS_FOR_AMD_GPUS)
    ## try finding ROCm SMI
    find_package(rocm_smi REQUIRED)
    target_link_libraries(${HWS_LIBRARY_NAME} PRIVATE -lrocm_smi64)
    target_include_directories(${HWS_LIBRARY_NAME} PRIVATE ${ROCM_SMI_INCLUDE_DIR})

    message(STATUS "Enable sampling of AMD GPU information using ROCm SMI.")

    # add source file to source file list
    target_sources(${HWS_LIBRARY_NAME} PRIVATE
            $<BUILD_INTERFACE:
            ${CMAKE_CURRENT_SOURCE_DIR}/src/hardware_sampling/gpu_amd/hardware_sampler.cpp;
            ${CMAKE_CURRENT_SOURCE_DIR}/src/hardware_sampling/gpu_amd/rocm_smi_samples.cpp
            >)

    # add compile definition
    target_compile_definitions(${HWS_LIBRARY_NAME} PUBLIC HWS_FOR_AMD_GPUS_ENABLED)
endif ()


####################################################################################################################
##                                        Intel GPU sampling via Level Zero                                       ##
####################################################################################################################
option(HWS_FOR_INTEL_GPUS "Enable hardware sampling for Intel GPUs" OFF)

if (HWS_FOR_INTEL_GPUS)
    # try finding Level Zero
    find_package(level_zero REQUIRED)
    target_link_libraries(${HWS_LIBRARY_NAME} PRIVATE level_zero)

    message(STATUS "Enable sampling of Intel GPU information using Level Zero.")

    # add source file to source file list
    target_sources(${HWS_LIBRARY_NAME} PRIVATE
            $<BUILD_INTERFACE:
            ${CMAKE_CURRENT_SOURCE_DIR}/src/hardware_sampling/gpu_intel/hardware_sampler.cpp;
            ${CMAKE_CURRENT_SOURCE_DIR}/src/hardware_sampling/gpu_intel/level_zero_samples.cpp;
            ${CMAKE_CURRENT_SOURCE_DIR}/src/hardware_sampling/gpu_intel/utility.cpp
            >)

    # add compile definition
    target_compile_definitions(${HWS_LIBRARY_NAME} PUBLIC HWS_FOR_INTEL_GPUS_ENABLED)
endif ()

# TODO: install target