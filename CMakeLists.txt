cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
add_compile_options("-Wno-attributes")

set(EXTRA_COMPONENT_DIRS "lib/CH422G" "lib/Config" "lib/LCD")

project(defender_dash)
