## Authors: Alexander Van Craen
## Copyright (C): 2024-today All Rights Reserved
## License: This file is released under the MIT license.
##          See the LICENSE.md file in the project root for full license information.
########################################################################################################################

## only build this backend's tests if it was actually compiled into ${HWS_LIBRARY_NAME}
get_target_property(HWS_COMPILE_DEFINITIONS ${HWS_LIBRARY_NAME} COMPILE_DEFINITIONS)

if ("HWS_FOR_AMD_GPUS_ENABLED" IN_LIST HWS_COMPILE_DEFINITIONS)
    ## hws links rocm_smi64/hip::host PRIVATEly (see src/hws/gpu_amd/CMakeLists.txt), so their include
    ## directories aren't propagated to consumers - re-find and re-link them here for the test executable
    find_package(rocm_smi REQUIRED)
    find_package(HIP REQUIRED)

    add_executable(hws_test_gpu_amd test.cpp)
    target_link_libraries(hws_test_gpu_amd PRIVATE ${HWS_LIBRARY_NAME} rocm_smi64 hip::host doctest::doctest_with_main)
    target_include_directories(hws_test_gpu_amd PRIVATE ${ROCM_SMI_INCLUDE_DIR})
    add_test(NAME gpu_amd COMMAND hws_test_gpu_amd)
    set_tests_properties(gpu_amd PROPERTIES TIMEOUT 30)
endif ()
