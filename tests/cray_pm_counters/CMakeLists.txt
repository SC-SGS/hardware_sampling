## Authors: Alexander Van Craen
## Copyright (C): 2024-today All Rights Reserved
## License: This file is released under the MIT license.
##          See the LICENSE.md file in the project root for full license information.
########################################################################################################################

# only build this backend's tests if it was actually compiled into ${HWS_LIBRARY_NAME}
get_target_property(HWS_COMPILE_DEFINITIONS ${HWS_LIBRARY_NAME} COMPILE_DEFINITIONS)

if ("HWS_FOR_CRAY_PM_COUNTERS_ENABLED" IN_LIST HWS_COMPILE_DEFINITIONS)
    add_executable(hws_test_cray_pm_counters test.cpp)
    target_link_libraries(hws_test_cray_pm_counters PRIVATE ${HWS_LIBRARY_NAME})
    add_test(NAME cray_pm_counters COMMAND hws_test_cray_pm_counters)
    set_tests_properties(cray_pm_counters PROPERTIES TIMEOUT 30)
endif ()
