/*
# requests.cpp

The "main" source file with most of the boilerplate code. Includes the
`NativesMain` header for initialising plugin-natives.

- `Supports` declares to the SA:MP server which features this plugin uses.
- `Load` is called when the plugin loads and sets up the `logprintf` function.
*/

#include <set>

#include <amx/amx.h>
#include <plugincommon.h>

#include "common.hpp"
#include "natives.hpp"
// #include "plugin-natives\NativesMain.hpp" // must be included last

logprintf_t logprintf;

extern "C" AMX_NATIVE_INFO amx_Natives[] = {
    { "RestfulClient", Natives::RestfulClient },
    { "RestfulGetData", Natives::RestfulGetData },
    { "RestfulPostData", Natives::RestfulPostData },
    { "RestfulGetJSON", Natives::RestfulGetJSON },
    { "RestfulPostJSON", Natives::RestfulPostJSON },
    { "RestfulHeaders", Natives::RestfulHeaders },

    { "JsonObject", Natives::JSON::Object },
    { "JsonInt", Natives::JSON::Int },
    { "JsonFloat", Natives::JSON::Float },
    { "JsonBool", Natives::JSON::Bool },
    { "JsonString", Natives::JSON::String },
    { "JsonArray", Natives::JSON::Array },

    { "JsonGetObject", Natives::JSON::GetObject },
    { "JsonGetInt", Natives::JSON::GetInt },
    { "JsonGetFloat", Natives::JSON::GetFloat },
    { "JsonGetBool", Natives::JSON::GetBool },
    { "JsonGetString", Natives::JSON::GetString },
    { "JsonGetArray", Natives::JSON::GetArray },
    { "JsonArrayObject", Natives::JSON::ArrayObject },

    { "JsonGetNodeInt", Natives::JSON::GetNodeInt },
    { "JsonGetNodeFloat", Natives::JSON::GetNodeFloat },
    { "JsonGetNodeBool", Natives::JSON::GetNodeBool },
    { "JsonGetNodeString", Natives::JSON::GetNodeString },

    { "JsonStringify", Natives::JSON::Stringify },
    { "JsonCleanup", Natives::JSON::Cleanup },
    { 0, 0 }
};

std::set<AMX*> amx_List;

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports()
{
    return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES | SUPPORTS_PROCESS_TICK;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void** ppData)
{
    pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
    logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];

    return true;
}

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX* amx)
{
    // return pawn_natives::AmxLoad(amx);
    amx_List.insert(amx);
    return amx_Register(amx, amx_Natives, -1);
}

PLUGIN_EXPORT void PLUGIN_CALL ProcessTick()
{
    for (AMX* i : amx_List) {
        Natives::processTick(i);
    }
}

PLUGIN_EXPORT int PLUGIN_CALL Unload()
{
    return 1;
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX* amx)
{
    amx_List.erase(amx);
    return 1;
}
