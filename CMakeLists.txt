cmake_minimum_required(VERSION 3.0)

project(restful)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/lib/cmake-modules")

if(UNIX)
	set(CMAKE_CXX_FLAGS "-m32 -fvisibility=hidden")
	set(CMAKE_C_FLAGS "-m32 -fvisibility=hidden")
	set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS OFF)
endif()

add_definitions(-D_NO_ASYNCRTIMP=1)
set(BUILD_SAMPLES OFF)
set(BUILD_TESTS OFF)
set(WERROR OFF)
set(CMAKE_BUILD_TYPE Release)
set(BUILD_SHARED_LIBS 0)
set(Boost_USE_STATIC_LIBS ON)
set(VCPKG_TARGET_ARCHITECTURE x86)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/test/plugins)

find_package(cpprestsdk REQUIRED NAMES cpprestsdk cpprest)
add_subdirectory(src)
