#include <set>

#include <amx/amx.h>
#include <plugincommon.h>

#include "common.hpp"
#include "natives.hpp"

logprintf_t logprintf;

extern "C" AMX_NATIVE_INFO amx_Natives[] = {
    {"RequestsClient", Natives::RequestsClient},
    {"RequestHeaders", Natives::RequestHeaders},
    {"Request", Natives::Request},
    {"RequestJSON", Natives::RequestJSON},

    {"WebSocketClient", Natives::WebSocketClient},
    {"WebSocketSend", Natives::WebSocketSend},
    {"JsonWebSocketClient", Natives::JsonWebSocketClient},
    {"JsonWebSocketSend", Natives::JsonWebSocketSend},

    {"JsonParse", Natives::JSON::Parse},
    {"JsonStringify", Natives::JSON::Stringify},
    {"JsonNodeType", Natives::JSON::NodeType},
    {"JsonObject", Natives::JSON::Object},
    {"JsonInt", Natives::JSON::Int},
    {"JsonFloat", Natives::JSON::Float},
    {"JsonBool", Natives::JSON::Bool},
    {"JsonString", Natives::JSON::String},
    {"JsonArray", Natives::JSON::Array},
    {"JsonAppend", Natives::JSON::Append},
    {"JsonSetObject", Natives::JSON::SetObject},
    {"JsonSetInt", Natives::JSON::SetInt},
    {"JsonSetFloat", Natives::JSON::SetFloat},
    {"JsonSetBool", Natives::JSON::SetBool},
    {"JsonSetString", Natives::JSON::SetString},
    {"JsonGetObject", Natives::JSON::GetObjectAlt}, // renamed due to a msvc macro interfering
    {"JsonGetInt", Natives::JSON::GetInt},
    {"JsonGetFloat", Natives::JSON::GetFloat},
    {"JsonGetBool", Natives::JSON::GetBool},
    {"JsonGetString", Natives::JSON::GetString},
    {"JsonGetArray", Natives::JSON::GetArray},
    {"JsonArrayLength", Natives::JSON::ArrayLength},
    {"JsonArrayObject", Natives::JSON::ArrayObject},

    {"JsonGetNodeInt", Natives::JSON::GetNodeInt},
    {"JsonGetNodeFloat", Natives::JSON::GetNodeFloat},
    {"JsonGetNodeBool", Natives::JSON::GetNodeBool},
    {"JsonGetNodeString", Natives::JSON::GetNodeString},

    {"JsonToggleGC", Natives::JSON::ToggleGC},
    {"JsonCleanup", Natives::JSON::Cleanup},

    {0, 0}};

std::set<AMX *> amx_List;

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports()
{
    return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES | SUPPORTS_PROCESS_TICK;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void **ppData)
{
    pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
    logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];

    return true;
}

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *amx)
{
    amx_List.insert(amx);
    return amx_Register(amx, amx_Natives, -1);
}

PLUGIN_EXPORT void PLUGIN_CALL ProcessTick()
{
    Natives::processTick(amx_List);
}

PLUGIN_EXPORT int PLUGIN_CALL Unload()
{
    return 1;
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *amx)
{
    amx_List.erase(amx);
    return 1;
}
