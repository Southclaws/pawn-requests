/*
# natives.cpp

This source file contains the bridge between natives and implementations. I
prefer to keep the actual implementation separate. The implementation contains
no instances of `cell` or `AMX*` and is purely C++ and external library code.
The code here acts as the translation between AMX data types and native types.
*/

#include "natives.hpp"
// #include "plugin-natives\NativeFunc.hpp"

// identToNode maps numeric identifiers to JSON node pointers.
std::unordered_map<int, web::json::value*> Natives::JSON::nodeTable;
int Natives::JSON::jsonPoolCounter = 0;

int Natives::RestfulGetData(AMX* amx, cell* params)
{
    return 0;
}

int Natives::RestfulPostData(AMX* amx, cell* params)
{
    return 0;
}

int Natives::RestfulGetJSON(AMX* amx, cell* params)
{
    return 0;
}

int Natives::RestfulPostJSON(AMX* amx, cell* params)
{
    return 0;
}

int Natives::RestfulHeaders(AMX* amx, cell* params)
{
    return 0;
}

// JSON implementation is directly in the Natives section unlike other API.
// this is purely to simplify things while working with JSON value types.

int Natives::JSON::Object(AMX* amx, cell* params)
{
    int arg = 0;
    std::string key;
    std::vector<std::pair<utility::string_t, web::json::value>> fields;

    for (int i = 1; i < params[0]; ++i) {
        if (params[i] == 0) {
            break;
        }

        cell* addr = nullptr;
        amx_GetAddr(amx, params[i], &addr);

        if (addr == nullptr) {
            break;
        }

        logprintf("arg: %d addr: %x value: %d", arg, addr, *addr);

        if (!key.empty()) {
            web::json::value obj = Get(*addr);
            if (obj == web::json::value::null()) {
                continue;
            }
            fields.push_back(std::make_pair(utility::conversions::to_string_t(key), obj));
            key = "";
        } else {
            if (*addr == 0) {
                break;
            }

            int len = 0;
            amx_StrLen(addr, &len);
            if (len <= 0 || len > 512) {
                logprintf("error: string length in Object out of bounds (%d)", len);
                return 0;
            }

            key = std::string(len, ' ');
            amx_GetString(&key[0], addr, 0, len + 1);
        }
    }

    web::json::value* obj = new web::json::value;
    *obj = web::json::value::object(fields);
    return Alloc(obj);
}

int Natives::JSON::String(AMX* amx, cell* params)
{
    web::json::value* obj = new web::json::value;
    *obj = web::json::value::string(utility::conversions::to_string_t(amx_GetCppString(amx, params[1])));
    return Alloc(obj);
}

int Natives::JSON::Int(AMX* amx, cell* params)
{
    web::json::value* obj = new web::json::value;
    *obj = web::json::value::number(params[1]);
    return Alloc(obj);
}

int Natives::JSON::Float(AMX* amx, cell* params)
{
    web::json::value* obj = new web::json::value;
    *obj = web::json::value::number(amx_ctof(params[1]));
    return Alloc(obj);
}

int Natives::JSON::Array(AMX* amx, cell* params)
{
    std::vector<web::json::value> fields;

    for (int i = 1; i < params[0]; ++i) {
        if (params[i] == 0) {
            break;
        }

        cell* addr = nullptr;
        amx_GetAddr(amx, params[i], &addr);

        if (addr == 0) {
            break;
        }

        auto obj = Get(*addr);
        fields.push_back(obj);
    }

    web::json::value* obj = new web::json::value;
    *obj = web::json::value::array(fields);
    return Alloc(obj);
}

int Natives::JSON::Stringify(AMX* amx, cell* params)
{
    auto obj = Get(params[1], false);
    std::string s = utility::conversions::to_utf8string(obj.serialize());

    amx_SetCppString(amx, params[2], s, params[3]);

    return 0;
}

int Natives::JSON::Cleanup(AMX* amx, cell* params)
{
    web::json::value* ptr = nodeTable[params[1]];
    if (ptr == nullptr) {
        logprintf("error: attempt to cleanup null ID %d", params[1]);
        return 1;
    }

    Erase(params[1]);

    return 0;
}

cell Natives::JSON::Alloc(web::json::value* item)
{
    int id = jsonPoolCounter++;
    nodeTable[id] = item;
    return id;
}

web::json::value Natives::JSON::Get(int id, bool gc)
{
    if (id < 0 || id > jsonPoolCounter) {
        logprintf("error: id %d out of range %d", id, jsonPoolCounter);
        return web::json::value::null();
    }

    web::json::value* ptr = nodeTable[id];
    if (ptr == nullptr) {
        logprintf("error: attempt to get node from null ID %d", id);
        return web::json::value::null();
    }

    // deref the node into a local copy for returning
    web::json::value copy = *ptr;
    if (gc) {
        // if gc, then delete the heap copy
        Erase(id);
    }
    // and return the copy
    return copy;
}

void Natives::JSON::Erase(int id)
{
    delete nodeTable[id];
    nodeTable[id] = nullptr;
}
