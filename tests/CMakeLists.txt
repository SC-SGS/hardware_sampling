## Authors: Alexander Van Craen
## Copyright (C): 2024-today All Rights Reserved
## License: This file is released under the MIT license.
##          See the LICENSE.md file in the project root for full license information.
########################################################################################################################

## fetch doctest, the same way fmt is fetched above: prefer an already installed version, fall back to FetchContent
set(HWS_doctest_VERSION 2.4.11)
find_package(doctest ${HWS_doctest_VERSION} QUIET)
if (doctest_FOUND)
    message(STATUS "Found package doctest.")
else ()
    message(STATUS "Couldn't find package doctest. Building version ${HWS_doctest_VERSION} from source.")
    # doctest 2.4.11's own CMakeLists.txt declares a cmake_minimum_required below 3.5, which CMake >= 4.0 refuses
    # to process at all; relax the floor just for this one fetch, then restore it so it doesn't leak elsewhere
    set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
    FetchContent_Declare(doctest
            GIT_REPOSITORY https://github.com/doctest/doctest.git
            GIT_TAG v${HWS_doctest_VERSION}
            QUIET
    )
    FetchContent_MakeAvailable(doctest)
    unset(CMAKE_POLICY_VERSION_MINIMUM)
endif ()

## one subdirectory per backend, mirroring the include/hws/<backend>/ and src/hws/<backend>/ layout; each
## subdirectory's CMakeLists.txt is responsible for only registering tests if its backend was actually compiled in

## tests for the backend-agnostic hws::detail helpers (include/hws/utility.hpp) - these compile unconditionally,
## regardless of which hardware backends are enabled, so they are never gated
add_subdirectory(detail)

add_subdirectory(gpu_nvidia)
add_subdirectory(gpu_amd)
add_subdirectory(gpu_intel)
