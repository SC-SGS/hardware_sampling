## Authors: Alexander Van Craen
## Copyright (C): 2024-today All Rights Reserved
## License: This file is released under the MIT license.
##          See the LICENSE.md file in the project root for full license information.
########################################################################################################################

## unlike the backend-specific test directories, this one is never gated: include/hws/utility.hpp's hws::detail
## helpers are header-only and compile into every configuration, regardless of which hardware backends are enabled
add_executable(hws_test_detail test.cpp)
target_link_libraries(hws_test_detail PRIVATE ${HWS_LIBRARY_NAME} doctest::doctest_with_main)
add_test(NAME detail COMMAND hws_test_detail)
# generously long on top of the usual 30s: the first real Windows CI run hit the 30s timeout with zero captured
# output at all (not even doctest's own startup banner) - raised to tell a genuine hang apart from a slow start
# (e.g. DLL loading) before investigating further
set_tests_properties(detail PROPERTIES TIMEOUT 120)
