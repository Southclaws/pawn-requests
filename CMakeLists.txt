cmake_minimum_required(VERSION 3.20)
project(requests)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR};${CMAKE_SOURCE_DIR}/lib/cmake-modules")
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

set(CONAN_CMAKE_SOURCE "${CMAKE_SOURCE_DIR}/lib/cmake-conan/conan.cmake")
set(CONAN_CMAKE_BINARY "${CMAKE_BINARY_DIR}/conan.cmake")
if(EXISTS "${CONAN_CMAKE_SOURCE}")
	configure_file("${CONAN_CMAKE_SOURCE}" "${CONAN_CMAKE_BINARY}" COPYONLY)
else()
	if(NOT EXISTS "${CONAN_CMAKE_BINARY}")
		message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
		file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.14/conan.cmake"
						  "${CONAN_CMAKE_BINARY}")
	endif()
endif()

include("${CONAN_CMAKE_BINARY}")

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
