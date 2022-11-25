#ifndef REQUESTS_NATIVES_H
#define REQUESTS_NATIVES_H

#include <set>
#include <string>
#include <utility>

#include <amx/amx2.h>
#include <cpprest/json.h>

#include "common.hpp"

namespace Natives
{
    int RequestsClient(AMX *amx, cell *params);
    int RequestHeaders(AMX *amx, cell *params);
    int Request(AMX *amx, cell *params);
    int RequestJSON(AMX *amx, cell *params);

    int WebSocketClient(AMX *amx, cell *params);
    int WebSocketSend(AMX *amx, cell *params);
    int JsonWebSocketClient(AMX *amx, cell *params);
    int JsonWebSocketSend(AMX *amx, cell *params);

    void processTick(std::set<AMX *> amx_List);

    namespace JSON
    {
        int Parse(AMX *amx, cell *params);
        int Stringify(AMX *amx, cell *params);
        int NodeType(AMX *amx, cell *params);

        int Object(AMX *amx, cell *params);
        int Bool(AMX *amx, cell *params);
        int Int(AMX *amx, cell *params);
        int Float(AMX *amx, cell *params);
        int String(AMX *amx, cell *params);
        int Array(AMX *amx, cell *params);
        int Append(AMX *amx, cell *params);
        int SetObject(AMX *amx, cell *params);
        int SetInt(AMX *amx, cell *params);
        int SetFloat(AMX *amx, cell *params);
        int SetBool(AMX *amx, cell *params);
        int SetString(AMX *amx, cell *params);
        int GetObjectAlt(AMX *amx, cell *params);
        int GetInt(AMX *amx, cell *params);
        int GetFloat(AMX *amx, cell *params);
        int GetBool(AMX *amx, cell *params);
        int GetString(AMX *amx, cell *params);
        int GetArray(AMX *amx, cell *params);
        int ArrayLength(AMX *amx, cell *params);
        int ArrayObject(AMX *amx, cell *params);

        int GetNodeInt(AMX *amx, cell *params);
        int GetNodeFloat(AMX *amx, cell *params);
        int GetNodeBool(AMX *amx, cell *params);
        int GetNodeString(AMX *amx, cell *params);

        int ToggleGC(AMX *amx, cell *params);
        int Cleanup(AMX *amx, cell *params);

        struct node
        {
            web::json::value *value;
            bool gc;
        };
        extern std::unordered_map<int, node> nodeTable;
        extern int jsonPoolCounter;
        int Alloc(web::json::value *item);
        web::json::value Get(int id, bool gc = true);
        web::json::value *GetPointer(int id);
        void Erase(int id);
    }
}

#endif
