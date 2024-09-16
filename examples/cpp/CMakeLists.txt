## Authors: Marcel Breyer
## Copyright (C): 2024-today All Rights Reserved
## License: This file is released under the MIT license.
##          See the LICENSE.md file in the project root for full license information.
########################################################################################################################

cmake_minimum_required(VERSION 3.22)

project(LibraryUsageExample LANGUAGES CXX)

find_package(hardware_sampling REQUIRED)

add_executable(prog main.cpp)

target_compile_features(prog PUBLIC cxx_std_17)
target_link_libraries(prog PUBLIC hws::hardware_sampling)