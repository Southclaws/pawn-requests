/*
# natives.cpp

This source file contains the bridge between natives and implementations. I
prefer to keep the actual implementation separate. The implementation contains
no instances of `cell` or `AMX*` and is purely C++ and external library code.
The code here acts as the translation between AMX data types and native types.
*/

#include "natives.hpp"
#include "impl.hpp"
// #include "plugin-natives\NativeFunc.hpp"

// identToNode maps numeric identifiers to JSON node pointers.
std::unordered_map<int, web::json::value*> Natives::JSON::nodeTable;
int Natives::JSON::jsonPoolCounter = 0;

int Natives::RequestsClient(AMX* amx, cell* params)
{
    std::string endpoint = amx_GetCppString(amx, params[1]);
    return Impl::RequestsClient(endpoint, params[2]);
}

int Natives::RequestHeaders(AMX* amx, cell* params)
{
    std::vector<std::pair<std::string, std::string>> headers;
    std::string key;
    for (size_t i = 1; i <= params[0] / sizeof(cell); i++) {
        std::string header = amx_GetCppString(amx, params[i]);
        if (i & 1) {
            key = header;
        } else {
            headers.push_back(std::make_pair(key, header));
        }
    }
    return Impl::RequestHeaders(headers);
}

int Natives::RequestText(AMX* amx, cell* params)
{
    int id = params[1];
    std::string path = amx_GetCppString(amx, params[2]);
    Impl::E_HTTP_METHOD method = static_cast<Impl::E_HTTP_METHOD>(params[3]);
    Impl::E_CONTENT_TYPE responseType = static_cast<Impl::E_CONTENT_TYPE>(params[4]);
    std::string callback = amx_GetCppString(amx, params[5]);
    char* data;
    amx_GetCString(amx, params[6], data);
    // std::string data = amx_GetCppString(amx, params[6]);
    int headers = params[7];

    return Impl::RequestText(id, path, method, responseType, callback, data, headers);
}

int Natives::RequestJSON(AMX* amx, cell* params)
{
    // int id = params[1];
    // std::string path = amx_GetCppString(amx, params[2]);
    // int method = params[3];
    // int responseType = params[4];
    // std::string callback = amx_GetCppString(amx, params[5]);
    // std::string data = amx_GetCppString(amx, params[6]);
    // int headers = params[7];

    // return Impl::RequestJSON(id, path, callback, method, responseType, callback, data, headers);
    return 0;
};

void Natives::processTick(AMX* amx)
{
    std::vector<Impl::ResponseData> responses = Impl::gatherResponses();
    for (auto response : responses) {
        logprintf("task acquired, %d", response.id);
        int error;
        int amx_idx;
        cell amx_addr;
        cell amx_ret;
        cell* phys_addr;

        error = amx_FindPublic(amx, response.callback.c_str(), &amx_idx);
        if (error != AMX_ERR_NONE) {
            logprintf("ERROR: failed to locate public function '%s' in amx, error: %d", response.callback.c_str(), error);
            continue;
        }

        switch (response.responseType) {
        case Impl::E_CONTENT_TYPE::string: {
            // (Request:id, E_HTTP_STATUS:status, data[], dataLen)
            amx_Push(amx, response.rawBody.length());
            amx_PushString(amx, &amx_addr, &phys_addr, response.rawBody.c_str(), 0, 0);
            amx_Push(amx, response.status);
            amx_Push(amx, response.id);

            amx_Exec(amx, &amx_ret, amx_idx);
            amx_Release(amx, amx_addr);

            break;
        }

        case Impl::E_CONTENT_TYPE::json: {
            json::value* obj = new json::value;
            *obj = json::value::parse(utility::conversions::to_string_t(response.rawBody));
            cell id = JSON::Alloc(obj);

            // (Request:id, E_HTTP_STATUS:status, Node:node)
            amx_Push(amx, id);
            amx_Push(amx, response.status);
            amx_Push(amx, response.id);

            amx_Exec(amx, &amx_ret, amx_idx);
            amx_Release(amx, amx_addr);

            JSON::Erase(id);
            break;
        }
        }
    }
}

// JSON implementation is directly in the Natives section unlike other API.
// this is purely to simplify things while working with JSON value types.

int Natives::JSON::Object(AMX* amx, cell* params)
{
    std::string key;
    std::vector<std::pair<utility::string_t, web::json::value>> fields;

    for (size_t i = 1; i <= params[0] / sizeof(cell); i++) {
        cell* addr = nullptr;
        amx_GetAddr(amx, params[i], &addr);

        if (addr == nullptr) {
            break;
        }

        if (i & 1) {
            int len = 0;
            amx_StrLen(addr, &len);
            if (len <= 0 || len > 512) {
                logprintf("error: string length in Object out of bounds (%d)", len);
                return -1;
            }

            key = std::string(len, ' ');
            amx_GetString(&key[0], addr, 0, len + 1);
        } else {
            web::json::value obj = Get(*addr);
            if (obj == web::json::value::null()) {
                logprintf("error: value node %d was invalid", *addr);
                return -2;
            }
            fields.push_back(std::make_pair(utility::conversions::to_string_t(key), obj));
        }
    }

    web::json::value* obj = new web::json::value;
    *obj = web::json::value::object(fields);
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

int Natives::JSON::Bool(AMX* amx, cell* params)
{
    web::json::value* obj = new web::json::value;
    *obj = web::json::value::boolean(params[1]);
    return Alloc(obj);
}

int Natives::JSON::String(AMX* amx, cell* params)
{
    web::json::value* obj = new web::json::value;
    *obj = web::json::value::string(utility::conversions::to_string_t(amx_GetCppString(amx, params[1])));
    return Alloc(obj);
}

int Natives::JSON::Array(AMX* amx, cell* params)
{
    std::vector<web::json::value> fields;

    for (size_t i = 1; i <= params[0] / sizeof(cell); i++) {
        cell* addr = nullptr;
        amx_GetAddr(amx, params[i], &addr);

        if (addr == nullptr) {
            break;
        }

        auto obj = Get(*addr);
        if (obj == web::json::value::null()) {
            logprintf("error: value node %d was invalid", *addr);
            return -2;
        }
        fields.push_back(obj);
    }

    web::json::value* obj = new web::json::value;
    *obj = web::json::value::array(fields);
    return Alloc(obj);
}

int Natives::JSON::GetObject(AMX* amx, cell* params)
{
    web::json::value obj = Get(params[1]);
    if (!obj.is_object()) {
        return 1;
    }

    std::string key = amx_GetCppString(amx, params[2]);

    web::json::value* result = new web::json::value();
    *result = obj.as_object()[utility::conversions::to_string_t(key)];
    cell id = Alloc(result);

    cell* addr = nullptr;
    amx_GetAddr(amx, params[3], &addr);
    *addr = id;

    return 0;
}

int Natives::JSON::GetInt(AMX* amx, cell* params)
{
    web::json::value obj = Get(params[1], false);
    std::string key = amx_GetCppString(amx, params[2]);
    if (!obj.is_object()) {
        return 1;
    }

    web::json::value target = obj.as_object().at(utility::conversions::to_string_t(key));
    if (!target.is_integer()) {
        return 2;
    }

    cell* addr = nullptr;
    amx_GetAddr(amx, params[3], &addr);
    *addr = target.as_integer();

    return 0;
}

int Natives::JSON::GetFloat(AMX* amx, cell* params)
{
    web::json::value obj = Get(params[1], false);
    std::string key = amx_GetCppString(amx, params[2]);
    if (!obj.is_object()) {
        return 1;
    }

    web::json::value target = obj.as_object().at(utility::conversions::to_string_t(key));
    if (!target.is_double()) {
        return 2;
    }

    cell* addr = nullptr;
    amx_GetAddr(amx, params[3], &addr);
    float d = static_cast<float>(target.as_double());
    *addr = amx_ftoc(d);

    return 0;
}

int Natives::JSON::GetBool(AMX* amx, cell* params)
{
    web::json::value obj = Get(params[1], false);
    std::string key = amx_GetCppString(amx, params[2]);
    if (!obj.is_object()) {
        return 1;
    }

    web::json::value target = obj.as_object().at(utility::conversions::to_string_t(key));
    if (!target.is_boolean()) {
        return 2;
    }

    cell* addr = nullptr;
    amx_GetAddr(amx, params[3], &addr);
    *addr = target.as_bool();

    return 0;
}

int Natives::JSON::GetString(AMX* amx, cell* params)
{
    web::json::value obj = Get(params[1], false);
    std::string key = amx_GetCppString(amx, params[2]);
    if (!obj.is_object()) {
        return 1;
    }

    web::json::value target = obj.as_object().at(utility::conversions::to_string_t(key));
    if (!target.is_string()) {
        return 2;
    }

    return amx_SetCppString(amx, params[3], utility::conversions::to_utf8string(target.as_string()).c_str(), params[4]);
}

int Natives::JSON::GetArray(AMX* amx, cell* params)
{
    web::json::value obj = Get(params[1], false);
    std::string key = amx_GetCppString(amx, params[2]);
    if (!obj.is_object()) {
        return 1;
    }

    web::json::value* target = new web::json::value;
    *target = obj.as_object().at(utility::conversions::to_string_t(key));
    if (!target->is_array()) {
        return 2;
    }

    cell* addr = nullptr;
    amx_GetAddr(amx, params[3], &addr);
    *addr = Alloc(target);

    return 0;
}

int Natives::JSON::ArrayObject(AMX* amx, cell* params)
{
    web::json::value obj = Get(params[1], false);
    if (!obj.is_array()) {
        return 1;
    }

    web::json::value* result = new web::json::value();
    *result = obj.as_array().at(params[2]);
    cell id = Alloc(result);

    cell* addr = nullptr;
    amx_GetAddr(amx, params[3], &addr);
    *addr = id;

    return 0;
}

int Natives::JSON::GetNodeInt(AMX* amx, cell* params)
{
    web::json::value obj = Get(params[1]);
    if (!obj.is_integer()) {
        return 1;
    }

    cell* addr = nullptr;
    amx_GetAddr(amx, params[2], &addr);
    *addr = obj.as_integer();

    return 0;
}

int Natives::JSON::GetNodeFloat(AMX* amx, cell* params)
{
    web::json::value obj = Get(params[1]);
    if (!obj.is_double()) {
        return 1;
    }

    cell* addr = nullptr;
    amx_GetAddr(amx, params[2], &addr);
    float d = static_cast<float>(obj.as_double());
    *addr = amx_ftoc(d);

    return 0;
}

int Natives::JSON::GetNodeBool(AMX* amx, cell* params)
{
    web::json::value obj = Get(params[1]);
    if (!obj.is_boolean()) {
        return 1;
    }

    cell* addr = nullptr;
    amx_GetAddr(amx, params[2], &addr);
    *addr = obj.as_bool();

    return 0;
}

int Natives::JSON::GetNodeString(AMX* amx, cell* params)
{
    web::json::value obj = Get(params[1]);
    if (!obj.is_string()) {
        return 1;
    }

    return amx_SetCppString(amx, params[2], utility::conversions::to_utf8string(obj.as_string()).c_str(), params[3]);
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
    nodeTable.erase(id);
}
