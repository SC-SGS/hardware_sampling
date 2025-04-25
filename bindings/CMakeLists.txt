## Authors: Marcel Breyer
## Copyright (C): 2024-today All Rights Reserved
## License: This file is released under the MIT license.
##          See the LICENSE.md file in the project root for full license information.
########################################################################################################################

message(STATUS "Building Python language bindings.")

find_package(Python COMPONENTS Interpreter Development)

## try finding pybind11
set(HWS_pybind11_VERSION master)
find_package(pybind11 2.13.1 QUIET)
if (pybind11_FOUND)
    message(STATUS "Found package pybind11.")
else ()
    message(STATUS "Couldn't find package pybind11. Building version ${HWS_pybind11_VERSION} from source.")
    # fetch pybind11 library for creating Python bindings
    FetchContent_Declare(pybind11
            GIT_REPOSITORY https://github.com/pybind/pybind11.git
            GIT_TAG ${HWS_pybind11_VERSION}
            GIT_SHALLOW TRUE
            QUIET
    )
    FetchContent_MakeAvailable(pybind11)
    target_include_directories(${HWS_LIBRARY_NAME} PUBLIC
            $<BUILD_INTERFACE:${pybind11_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )
endif ()

# set source files that are always used
set(HWS_PYTHON_BINDINGS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/event.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/relative_event.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware_sampler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/sample_category.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/system_hardware_sampler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/version.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

# add hardware sampling specific source files if the respective sampling is used
get_target_property(HWS_COMPILE_DEFINITIONS ${HWS_LIBRARY_NAME} COMPILE_DEFINITIONS)
if ("HWS_FOR_CPUS_ENABLED" IN_LIST HWS_COMPILE_DEFINITIONS)
    list(APPEND HWS_PYTHON_BINDINGS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/cpu_hardware_sampler.cpp)
endif ()
if ("HWS_FOR_NVIDIA_GPUS_ENABLED" IN_LIST HWS_COMPILE_DEFINITIONS)
    list(APPEND HWS_PYTHON_BINDINGS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/gpu_nvidia_hardware_sampler.cpp)
endif ()
if ("HWS_FOR_AMD_GPUS_ENABLED" IN_LIST HWS_COMPILE_DEFINITIONS)
    list(APPEND HWS_PYTHON_BINDINGS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/gpu_amd_hardware_sampler.cpp)
endif ()
if ("HWS_FOR_INTEL_GPUS_ENABLED" IN_LIST HWS_COMPILE_DEFINITIONS)
    list(APPEND HWS_PYTHON_BINDINGS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/gpu_intel_hardware_sampler.cpp)
endif ()

# create pybind11 module
set(HWS_PYTHON_BINDINGS_LIBRARY_NAME HardwareSampling)
pybind11_add_module(${HWS_PYTHON_BINDINGS_LIBRARY_NAME} ${HWS_PYTHON_BINDINGS_SOURCES})

# add necessary compile options
target_include_directories(${HWS_PYTHON_BINDINGS_LIBRARY_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/..)
target_link_libraries(${HWS_PYTHON_BINDINGS_LIBRARY_NAME} PRIVATE ${HWS_LIBRARY_NAME})
target_compile_definitions(${HWS_PYTHON_BINDINGS_LIBRARY_NAME} PRIVATE PYBIND11_DETAILED_ERROR_MESSAGES)

include(GNUInstallDirs)
# install Python bindings
install(TARGETS ${HWS_PYTHON_BINDINGS_LIBRARY_NAME}
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"  # all shared lib files
)