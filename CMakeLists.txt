## Authors: Marcel Breyer
## Copyright (C): 2024-today All Rights Reserved
## License: This file is released under the MIT license.
##          See the LICENSE.md file in the project root for full license information.
########################################################################################################################

cmake_minimum_required(VERSION 3.22)

project("hws - Hardware Sampling for GPUs and CPUs"
        VERSION 1.1.0
        LANGUAGES CXX
        DESCRIPTION "Hardware sampling (e.g., clock frequencies, memory consumption, temperatures, or energy draw) for CPUs and GPUS.")

# explicitly set library source files
set(HWS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/hws/event.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/hws/hardware_sampler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/hws/system_hardware_sampler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/hws/utility.cpp
)

# create hardware sampling library
set(HWS_LIBRARY_NAME hws)
add_library(${HWS_LIBRARY_NAME} SHARED ${HWS_SOURCES})
add_library(hws::hws ALIAS ${HWS_LIBRARY_NAME})

# set install target
set(HWS_TARGETS_TO_INSTALL )

# use C++17
target_compile_features(${HWS_LIBRARY_NAME} PUBLIC cxx_std_17)

# add target include directory
target_include_directories(${HWS_LIBRARY_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

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

# install fmt as dependency
include(FetchContent)
set(HWS_fmt_VERSION 11.0.2)
find_package(fmt 11.0.2 QUIET)
if (fmt_FOUND)
    message(STATUS "Found package fmt.")
else ()
    message(STATUS "Couldn't find package fmt. Building version ${HWS_fmt_VERSION} from source.")
    set(FMT_PEDANTIC OFF CACHE INTERNAL "" FORCE)
    set(FMT_WERROR OFF CACHE INTERNAL "" FORCE)
    set(FMT_DOC OFF CACHE INTERNAL "" FORCE)
    set(FMT_INSTALL ON CACHE INTERNAL "" FORCE) # let {fmt} handle the install target
    set(FMT_TEST OFF CACHE INTERNAL "" FORCE)
    set(FMT_FUZZ OFF CACHE INTERNAL "" FORCE)
    set(FMT_CUDA_TEST OFF CACHE INTERNAL "" FORCE)
    set(FMT_MODULE OFF CACHE INTERNAL "" FORCE)
    set(FMT_SYSTEM_HEADERS ON CACHE INTERNAL "" FORCE)
    # fetch string formatting library fmt
    FetchContent_Declare(fmt
            GIT_REPOSITORY https://github.com/fmtlib/fmt.git
            GIT_TAG ${HWS_fmt_VERSION}
            QUIET
    )
    FetchContent_MakeAvailable(fmt)
    set_property(TARGET fmt PROPERTY POSITION_INDEPENDENT_CODE ON)
    add_dependencies(${HWS_LIBRARY_NAME} fmt)
endif ()
target_link_libraries(${HWS_LIBRARY_NAME} PUBLIC fmt::fmt)

########################################################################################################################
##                                              configure version header                                              ##
########################################################################################################################
message(STATUS "Configuring version information.")
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/include/hws/version.hpp.in
        ${CMAKE_CURRENT_SOURCE_DIR}/include/hws/version.hpp
        @ONLY
)

####################################################################################################################
##                                                CPU measurements                                                ##
####################################################################################################################
set(HWS_ENABLE_CPU_SAMPLING AUTO CACHE STRING "Enable sampling of CPUs.")
set_property(CACHE HWS_ENABLE_CPU_SAMPLING PROPERTY STRINGS AUTO ON OFF)
if (HWS_ENABLE_CPU_SAMPLING MATCHES "AUTO" OR HWS_ENABLE_CPU_SAMPLING)
    add_subdirectory(src/hws/cpu)
else ()
    message(STATUS "Hardware sampling for CPUs disabled!")
endif ()


####################################################################################################################
##                                          NVIDIA GPU sampling via NVML                                          ##
####################################################################################################################
set(HWS_ENABLE_GPU_NVIDIA_SAMPLING AUTO CACHE STRING "Enable sampling of NVIDIA GPUs.")
set_property(CACHE HWS_ENABLE_GPU_NVIDIA_SAMPLING PROPERTY STRINGS AUTO ON OFF)
if (HWS_ENABLE_GPU_NVIDIA_SAMPLING MATCHES "AUTO" OR HWS_ENABLE_GPU_NVIDIA_SAMPLING)
    add_subdirectory(src/hws/gpu_nvidia)
else ()
    message(STATUS "Hardware sampling for NVIDIA GPUs disabled!")
endif ()


####################################################################################################################
##                                        AMD GPU sampling via ROCm SMI lib                                       ##
####################################################################################################################
set(HWS_ENABLE_GPU_AMD_SAMPLING AUTO CACHE STRING "Enable sampling of AMD GPUs.")
set_property(CACHE HWS_ENABLE_GPU_AMD_SAMPLING PROPERTY STRINGS AUTO ON OFF)
if (HWS_ENABLE_GPU_AMD_SAMPLING MATCHES "AUTO" OR HWS_ENABLE_GPU_AMD_SAMPLING)
    add_subdirectory(src/hws/gpu_amd)
else ()
    message(STATUS "Hardware sampling for AMD GPUs disabled!")
endif ()


####################################################################################################################
##                                        Intel GPU sampling via Level Zero                                       ##
####################################################################################################################
set(HWS_ENABLE_GPU_INTEL_SAMPLING AUTO CACHE STRING "Enable sampling of Intel GPUs.")
set_property(CACHE HWS_ENABLE_GPU_INTEL_SAMPLING PROPERTY STRINGS AUTO ON OFF)
if (HWS_ENABLE_GPU_INTEL_SAMPLING MATCHES "AUTO" OR HWS_ENABLE_GPU_INTEL_SAMPLING)
    add_subdirectory(src/hws/gpu_intel)
else ()
    message(STATUS "Hardware sampling for Intel GPUs disabled!")
endif ()


####################################################################################################################
##                                             enable Python bindings                                             ##
####################################################################################################################
option(HWS_ENABLE_PYTHON_BINDINGS "Build language bindings for Python." ON)
if (HWS_ENABLE_PYTHON_BINDINGS)
    add_subdirectory(bindings)
endif ()


########################################################################################################################
##                                                  add documentation                                                 ##
########################################################################################################################
option(HWS_ENABLE_DOCUMENTATION "Add documentation using Doxygen." OFF)
if (HWS_ENABLE_DOCUMENTATION)
    add_subdirectory(docs)
endif ()


########################################################################################################################
##                                           add support for `make install`                                           ##
########################################################################################################################
include(GNUInstallDirs)
## install all necessary library targets
install(TARGETS ${HWS_LIBRARY_NAME}
        EXPORT hws_Targets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"  # all files that are neither executables, shared lib or headers
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"  # all shared lib files
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"  # all executables
)

## mark header to install via 'make install'
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

## manage version comparison
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        "hwsConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)

## generate configuration file
configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/hwsConfig.cmake.in"
        "${PROJECT_BINARY_DIR}/hwsConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/hws/cmake
)

## create and copy install-targets file
install(EXPORT hws_Targets
        FILE hwsTargets.cmake
        NAMESPACE hws::
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/hws/cmake
)

## create file containing the build configuration and version information
install(FILES
        "${PROJECT_BINARY_DIR}/hwsConfig.cmake"
        "${PROJECT_BINARY_DIR}/hwsConfigVersion.cmake"
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Findlevel_zero.cmake"
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/hws/cmake
)