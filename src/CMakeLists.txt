include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(NO_OUTPUT_DIRS)

include(AMXConfig)
include(AddSAMPPlugin)

set(SAMP_SDK_ROOT "${CMAKE_SOURCE_DIR}/lib/samp-plugin-sdk")
find_package(SAMPSDK REQUIRED)

include_directories(
	${SAMPSDK_INCLUDE_DIR}
)

add_samp_plugin(requests
	${SAMPSDK_DIR}/amxplugin.cpp
	${SAMPSDK_DIR}/amxplugin2.cpp
	${SAMPSDK_DIR}/amx/getch.c
	common.hpp
	requests.cpp
	impl.cpp
	impl.hpp
	natives.cpp
	natives.hpp
	plugin.def
)

if(MSVC)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
	target_compile_definitions(requests PRIVATE
		_CRT_SECURE_NO_WARNINGS
		NOMINMAX
		_WIN32_WINNT=0x0600
		_SCL_SECURE_NO_WARNINGS
	)

	target_compile_options(requests PRIVATE
		/W4
		/permissive-
	)

	# also enable multi-processor compilation
	target_compile_options(requests PRIVATE
		/MP
	)
endif()

if (MSVC)
add_custom_command(
    TARGET requests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/lib/requests.dll
		${CMAKE_SOURCE_DIR}/test/plugins/requests.dll)
endif()

conan_target_link_libraries(requests)
