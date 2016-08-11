cmake_minimum_required(VERSION 2.8)

project(libsuperderpy C)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

include(libsuperderpy)
include(SetPaths)

add_subdirectory(src)

