cmake_minimum_required(VERSION 3.0)

project(restful)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/lib/cmake-modules")

if(UNIX)
	set(CMAKE_CXX_FLAGS "-m32 -std=c++17 -fvisibility=hidden")
	set(CMAKE_C_FLAGS "-m32 -std=c++17 -fvisibility=hidden")
	set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS OFF)
endif()

# set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/test/plugins)
# set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
# set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(BUILD_CURL_EXE OFF)
set(CURL_STATICLIB ON)
set(HTTP_ONLY ON)
set(CMAKE_USE_LIBSSH2 OFF)
set(ENABLE_MANUAL OFF)
set(BUILD_TESTING OFF)
set(OPENSSL_ROOT_DIR "${CMAKE_SOURCE_DIR}/lib/openssl")

add_subdirectory(lib/curl)
add_subdirectory(src)
