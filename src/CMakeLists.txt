include(AMXConfig)
include(AddSAMPPlugin)

set(SAMP_SDK_ROOT "${PROJECT_SOURCE_DIR}/lib/samp-plugin-sdk")
find_package(SAMPSDK REQUIRED)

include_directories(
	${SAMPSDK_INCLUDE_DIR}
)

add_samp_plugin(restful
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
	restful.cpp
	impl.cpp
	impl.hpp
	natives.cpp
	natives.hpp
)
