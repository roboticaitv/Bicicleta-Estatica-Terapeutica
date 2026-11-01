cmake_minimum_required(VERSION 3.22)

set(SDKCONFIG "${CMAKE_SOURCE_DIR}/build/sdkconfig")
set(SDKCONFIG_DEFAULTS "${CMAKE_SOURCE_DIR}/build/sdkconfig.defaults")

set(EXTRA_COMPONENT_DIRS
    "${CMAKE_SOURCE_DIR}/apps"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(biciver)
