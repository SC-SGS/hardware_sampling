## Authors: Marcel Breyer
## Copyright (C): 2024-today All Rights Reserved
## License: This file is released under the MIT license.
##          See the LICENSE.md file in the project root for full license information.
########################################################################################################################

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

# check of any CPU related utility could be found
if (NOT (HWS_LSCPU_FOUND OR HWS_FREE_FOUND OR HWS_TURBOSTAT_EXECUTION_TYPE))
    if (HWS_ENABLE_CPU_SAMPLING MATCHES "ON")
        message(SEND_ERROR "Cannot find any CPU utility program but CPU sampling was explicitly requested!")
    endif ()
    return()
endif ()
message(STATUS "Enable sampling of CPU information.")

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
    )
    FetchContent_MakeAvailable(subprocess)
    target_include_directories(${HWS_LIBRARY_NAME} PRIVATE $<BUILD_INTERFACE:${subprocess_SOURCE_DIR}>)
endif ()

# add source file to source file list
target_sources(${HWS_LIBRARY_NAME} PRIVATE
        $<BUILD_INTERFACE:
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware_sampler.cpp;
        ${CMAKE_CURRENT_SOURCE_DIR}/cpu_samples.cpp;
        ${CMAKE_CURRENT_SOURCE_DIR}/utility.cpp;
        >)

# add compile definitions
target_compile_definitions(${HWS_LIBRARY_NAME} PUBLIC HWS_FOR_CPUS_ENABLED)
