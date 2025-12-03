cmake_minimum_required(VERSION 3.20)
project(requests)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR};${CMAKE_SOURCE_DIR}/lib/cmake-modules")
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

set(CONAN_CMAKE_FILE "${CMAKE_BINARY_DIR}/conan.cmake")
if(NOT EXISTS "${CONAN_CMAKE_FILE}")
	message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
	file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.14/conan.cmake"
					  "${CONAN_CMAKE_FILE}")
endif()

file(READ "${CONAN_CMAKE_FILE}" _conan_helper_contents)
set(_conan_msvc_block "    elseif(NOT MSVC_VERSION VERSION_LESS 1930 AND MSVC_VERSION VERSION_LESS 1940)\n        set(\${result} 17 PARENT_SCOPE)\n    else()")
set(_conan_msvc_patch "    elseif(NOT MSVC_VERSION VERSION_LESS 1930 AND MSVC_VERSION VERSION_LESS 1940)\n        set(\${result} 17 PARENT_SCOPE)\n    elseif(NOT MSVC_VERSION VERSION_LESS 1940 AND MSVC_VERSION VERSION_LESS 1950)\n        set(\${result} 17 PARENT_SCOPE)\n    else()")
if(_conan_helper_contents MATCHES "VERSION_LESS 1930" AND NOT _conan_helper_contents MATCHES "VERSION_LESS 1950")
	string(REPLACE "${_conan_msvc_block}" "${_conan_msvc_patch}" _conan_helper_patched "${_conan_helper_contents}")
	if(NOT _conan_helper_patched STREQUAL _conan_helper_contents)
		file(WRITE "${CONAN_CMAKE_FILE}" "${_conan_helper_patched}")
	endif()
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
