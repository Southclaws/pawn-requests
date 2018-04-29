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
	plugin-natives/NativeFunc.hpp
	plugin-natives/NativeHook.hpp
	plugin-natives/NativeImport.hpp
	plugin-natives/NativesMain.hpp
	plugin-natives/Internal/NativeCast.hpp
	plugin-natives/Internal/NativeFuncImpl.hpp
	plugin-natives/Internal/NativeHookImpl.hpp
	common.hpp
	requests.cpp
	impl.cpp
	impl.hpp
	natives.cpp
	natives.hpp
	plugin.def
)

target_link_libraries(requests cpprestsdk::cpprest)

if(UNIX)
target_link_libraries(requests ${Boost_LIBRARIES})
endif()
