## Authors: Alexander Van Craen
## Copyright (C): 2024-today All Rights Reserved
## License: This file is released under the MIT license.
##          See the LICENSE.md file in the project root for full license information.
########################################################################################################################

## one subdirectory per backend, mirroring the include/hws/<backend>/ and src/hws/<backend>/ layout; each
## subdirectory's CMakeLists.txt is responsible for only registering tests if its backend was actually compiled in
add_subdirectory(cray_pm_counters)
