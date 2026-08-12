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

## the turbostat "-i" measurement interval, in seconds
## NOTE: very short intervals (e.g. this option's default of 0.001, kept for backwards
## compatibility with the previous hardcoded value and any sudoers/paper-reproducibility setups
## that already depend on it) have been observed to produce physically implausible readings
## (multi-kW package power, multi-GHz core clocks) on heavily loaded, many-core machines,
## especially with older turbostat builds -- turbostat's own internal timing calculation degrades
## under load for short intervals. 1 second was the smallest value that stayed reliable across
## every machine/turbostat-version combination we tested; consider overriding this to "1" unless
## you specifically need to match the old default's exact (sudoers-permitted) command line.
set(HWS_TURBOSTAT_INTERVAL "0.001" CACHE STRING "The interval in seconds ('sec.subsec', passed to turbostat's -i flag) turbostat itself measures over for a single CPU power/frequency sample.")
if (NOT ${HWS_TURBOSTAT_INTERVAL} MATCHES "^[0-9]+(\\.[0-9]+)?$" OR ${HWS_TURBOSTAT_INTERVAL} EQUAL 0)
    message(FATAL_ERROR "The HWS_TURBOSTAT_INTERVAL must be a positive number (e.g. \"1\" or \"0.5\"), but is \"${HWS_TURBOSTAT_INTERVAL}\"!")
endif ()
message(STATUS "Setting the turbostat measurement interval to ${HWS_TURBOSTAT_INTERVAL}s.")

## check whether turbostat could be found -> used for the CPU targets as well as for ALL host measurements
## -> checked even if no CPU targets where provided
find_program(HWS_TURBOSTAT_FOUND turbostat)
if (HWS_TURBOSTAT_FOUND)
    ## check if the turbostat command works as intended
    execute_process(COMMAND sudo -n turbostat -n 1 -i ${HWS_TURBOSTAT_INTERVAL} -S -q
            RESULT_VARIABLE HWS_TURBOSTAT_WITH_ROOT_N
            OUTPUT_QUIET
            ERROR_QUIET)
    if (HWS_TURBOSTAT_WITH_ROOT_N EQUAL 0)
        ## can execute with root
        execute_process(COMMAND sudo turbostat -n 1 -i ${HWS_TURBOSTAT_INTERVAL} -S -q
                RESULT_VARIABLE HWS_TURBOSTAT_WITH_ROOT
                OUTPUT_QUIET
                ERROR_QUIET)
        if (HWS_TURBOSTAT_WITH_ROOT EQUAL 0)
            message(STATUS "Enable sampling of CPU information using turbostat with root privileges.")

            set(HWS_TURBOSTAT_EXECUTION_TYPE "root")
            # add compile definitions
            target_compile_definitions(${HWS_LIBRARY_NAME} PUBLIC HWS_VIA_TURBOSTAT_ENABLED)
            target_compile_definitions(${HWS_LIBRARY_NAME} PUBLIC HWS_VIA_TURBOSTAT_ROOT)
        else ()
            message(STATUS "Can't use turbostat even with root privileges!")
            message(STATUS "Disabling turbostat support!")
        endif ()
    else ()
        ## check if turbostat can be executed without root -> potential less data
        execute_process(COMMAND turbostat -n 1 -i ${HWS_TURBOSTAT_INTERVAL} -S -q
                RESULT_VARIABLE HWS_TURBOSTAT_WITHOUT_ROOT
                OUTPUT_QUIET
                ERROR_QUIET)
        if (HWS_TURBOSTAT_WITHOUT_ROOT EQUAL 0)
            message(STATUS "Enable sampling of CPU information using turbostat without root privileges.")

            set(HWS_TURBOSTAT_EXECUTION_TYPE "without_root")
            # add compile definitions
            target_compile_definitions(${HWS_LIBRARY_NAME} PUBLIC HWS_VIA_TURBOSTAT_ENABLED)
        else ()
            message(STATUS "Can't use turbostat (with or without root privileges)!")
            message(STATUS "Disabling turbostat support!")
        endif ()
    endif ()

    if (HWS_TURBOSTAT_EXECUTION_TYPE)
        target_compile_definitions(${HWS_LIBRARY_NAME} PUBLIC HWS_TURBOSTAT_INTERVAL="${HWS_TURBOSTAT_INTERVAL}")
    endif ()
endif ()

# check of any CPU related utility could be found
if (NOT (HWS_LSCPU_FOUND OR HWS_FREE_FOUND OR HWS_TURBOSTAT_EXECUTION_TYPE))
    if (HWS_ENABLE_CPU_SAMPLING MATCHES "ON")
        message(SEND_ERROR "Cannot find any CPU utility program but CPU sampling was explicitly requested!")
    else ()
        message(STATUS "Cannot find any CPU utility program. Hardware sampling for CPUs disabled.")
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
