/*
# restful.cpp

The "main" source file with most of the boilerplate code. Includes the
`NativesMain` header for initialising plugin-natives.

- `Supports` declares to the SA:MP server which features this plugin uses.
- `Load` is called when the plugin loads and sets up the `logprintf` function.
*/

#include <amx/amx.h>
#include <plugincommon.h>

#include "common.hpp"
#include "natives.hpp"
#include "plugin-natives\NativesMain.hpp" // must be included last

logprintf_t logprintf;

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports()
{
    return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void** ppData)
{
    pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
    logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];
    return true;
}

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *amx)
{
	return pawn_natives::AmxLoad(amx);
}

PLUGIN_EXPORT int PLUGIN_CALL Unload()
{
	return 1;
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload()
{
	return 1;
}
