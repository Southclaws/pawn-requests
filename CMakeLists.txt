cmake_minimum_required(VERSION 3.0)

project(restful)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/lib/cmake-modules")

if(UNIX)
	set(CMAKE_CXX_FLAGS "-m32 -fvisibility=hidden")
	set(CMAKE_C_FLAGS "-m32 -fvisibility=hidden")
	set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS OFF)
endif()

set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/test/plugins)

find_package(cpprestsdk REQUIRED)
add_subdirectory(src)
