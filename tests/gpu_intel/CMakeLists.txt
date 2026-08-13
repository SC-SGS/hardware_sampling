## Authors: Alexander Van Craen
## Copyright (C): 2024-today All Rights Reserved
## License: This file is released under the MIT license.
##          See the LICENSE.md file in the project root for full license information.
########################################################################################################################

## only build this backend's tests if it was actually compiled into ${HWS_LIBRARY_NAME}
get_target_property(HWS_COMPILE_DEFINITIONS ${HWS_LIBRARY_NAME} COMPILE_DEFINITIONS)

if ("HWS_FOR_INTEL_GPUS_ENABLED" IN_LIST HWS_COMPILE_DEFINITIONS)
    ## hws links level_zero PRIVATEly (see src/hws/gpu_intel/CMakeLists.txt), so its include directories aren't
    ## propagated to consumers - re-find and re-link it here for the test executable
    list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
    find_package(level_zero REQUIRED)

    add_executable(hws_test_gpu_intel test.cpp)
    target_link_libraries(hws_test_gpu_intel PRIVATE ${HWS_LIBRARY_NAME} level_zero doctest::doctest_with_main)
    add_test(NAME gpu_intel COMMAND hws_test_gpu_intel)
    set_tests_properties(gpu_intel PROPERTIES TIMEOUT 30)
endif ()
