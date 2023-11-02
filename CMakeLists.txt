cmake_minimum_required(VERSION 3.6...3.27)

project(libsuperderpy C)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

include(libsuperderpy)

add_subdirectory(src)
add_subdirectory(test)
