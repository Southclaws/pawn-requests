cmake_minimum_required(VERSION 3.20)
project(requests)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR};${CMAKE_SOURCE_DIR}/lib/cmake-modules")
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

if(WIN32 AND NOT DEFINED ENV{CONAN_VCVARS_VER})
	set(ENV{CONAN_VCVARS_VER} "14.4")
endif()

set(CONAN_CMAKE_FILE "${CMAKE_SOURCE_DIR}/lib/cmake-conan/conan.cmake")
if(NOT EXISTS "${CONAN_CMAKE_FILE}")
	message(FATAL_ERROR "Missing ${CONAN_CMAKE_FILE}; ensure the vendored conan.cmake is present in the repository.")
endif()

include("${CONAN_CMAKE_FILE}")

conan_cmake_run(REQUIRES cpprestsdk/2.10.18
               BASIC_SETUP BUILD missing
               SETTINGS arch=x86)

set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/test/plugins)

if (MSVC)
	# link runtime statically
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
elseif(UNIX)
	# hide non-exported symbols
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -fvisibility=hidden")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -fvisibility=hidden")
	set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS OFF)

	# link runtime statically
	set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
	add_link_options(
		"-static-libgcc"
		"-static-libstdc++"
	)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath='$ORIGIN'")
endif()

add_subdirectory(src)
