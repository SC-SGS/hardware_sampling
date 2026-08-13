## Authors: Alexander Van Craen
## Copyright (C): 2024-today All Rights Reserved
## License: This file is released under the MIT license.
##          See the LICENSE.md file in the project root for full license information.
########################################################################################################################

## only build this backend's tests if it was actually compiled into ${HWS_LIBRARY_NAME}
get_target_property(HWS_COMPILE_DEFINITIONS ${HWS_LIBRARY_NAME} COMPILE_DEFINITIONS)

if ("HWS_FOR_NVIDIA_GPUS_ENABLED" IN_LIST HWS_COMPILE_DEFINITIONS)
    ## hws links CUDA::nvml/CUDA::cudart PRIVATEly (see src/hws/gpu_nvidia/CMakeLists.txt), so their include
    ## directories aren't propagated to consumers - re-find and re-link them here for the test executable
    find_package(CUDAToolkit REQUIRED)

    add_executable(hws_test_gpu_nvidia test.cpp)
    target_link_libraries(hws_test_gpu_nvidia PRIVATE ${HWS_LIBRARY_NAME} CUDA::nvml CUDA::cudart doctest::doctest_with_main)
    add_test(NAME gpu_nvidia COMMAND hws_test_gpu_nvidia)
    set_tests_properties(gpu_nvidia PROPERTIES TIMEOUT 30)
endif ()
