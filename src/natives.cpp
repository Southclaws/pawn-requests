#include "natives.hpp"
#include "impl.hpp"
// #include "plugin-natives\NativeFunc.hpp"

// nodeTable maps numeric identifiers to JSON node pointers.
std::unordered_map<int, Natives::JSON::node> Natives::JSON::nodeTable;
int Natives::JSON::jsonPoolCounter = 0;

int Natives::RequestsClient(AMX *amx, cell *params)
{
    std::string endpoint = amx_GetCppString(amx, params[1]);
    return Impl::RequestsClient(endpoint, params[2]);
}

int Natives::RequestHeaders(AMX *amx, cell *params)
{
    std::vector<std::pair<std::string, std::string>> headers;
    std::string key;
    for (size_t i = 1; i <= params[0] / sizeof(cell); i++)
    {
        std::string header = amx_GetCppString(amx, params[i]);
        if (i & 1)
        {
            key = header;
        }
        else
        {
            headers.push_back(std::make_pair(key, header));
        }
    }
    return Impl::RequestHeaders(headers);
}

int Natives::Request(AMX *amx, cell *params)
{
    int id = params[1];
    std::string path = amx_GetCppString(amx, params[2]);
    Impl::E_HTTP_METHOD method = static_cast<Impl::E_HTTP_METHOD>(params[3]);
    std::string callback = amx_GetCppString(amx, params[4]);
    char *data;
    amx_GetCString(amx, params[5], data);
    // std::string data = amx_GetCppString(amx, params[6]);
    int headers = params[6];

    return Impl::Request(amx, id, path, method, callback, data, headers);
}

int Natives::RequestJSON(AMX *amx, cell *params)
{
    int id = params[1];
    std::string path = amx_GetCppString(amx, params[2]);
    Impl::E_HTTP_METHOD method = static_cast<Impl::E_HTTP_METHOD>(params[3]);
    std::string callback = amx_GetCppString(amx, params[4]);
    auto obj = JSON::Get(params[5]);
    int headers = params[6];

    return Impl::RequestJSON(amx, id, path, method, callback, obj, headers);
}

void Natives::processTick(std::set<AMX *> amx_List)
{
    std::vector<Impl::ResponseData> responses = Impl::gatherResponses();
    for (auto response : responses)
    {
        AMX *currentAmx = nullptr;
        for (AMX *amx : amx_List)
        {
            if (amx != response.amx)
            {
                continue;
            }

            currentAmx = amx;
            break;
        }

        if (currentAmx == nullptr)
        {
            return;
        }

        int error;
        int amx_idx;
        cell amx_addr;
        cell amx_ret;
        cell *phys_addr;

        error = amx_FindPublic(currentAmx, response.callback.c_str(), &amx_idx);
        if (error != AMX_ERR_NONE)
        {
            logprintf("ERROR: failed to locate public function '%s' in amx, error: %d", response.callback.c_str(), error);
            continue;
        }

        switch (response.responseType)
        {
        default:
        {
            logprintf("ERROR: Invalid response object type: %d", response.responseType);
            break;
        }
        case Impl::E_CONTENT_TYPE::empty:
        {
            // (Request:id, errorCode, errorMessage[], len)
            amx_Push(currentAmx, response.rawBody.length());
            amx_PushString(currentAmx, &amx_addr, &phys_addr, response.rawBody.c_str(), 0, 0);
            amx_Push(currentAmx, response.status);
            amx_Push(currentAmx, response.id);

            amx_Exec(currentAmx, &amx_ret, amx_idx);
            amx_Release(currentAmx, amx_addr);

            break;
        }

        case Impl::E_CONTENT_TYPE::string:
        {
            amx_Push(currentAmx, response.rawBody.length());
            amx_PushString(currentAmx, &amx_addr, &phys_addr, response.rawBody.c_str(), 0, 0);

            // signature is either
            // (Request:id, E_HTTP_STATUS:status, data[], dataLen)
            // or:
            // (WebSocket:id, data[], dataLen)
            // depending on whether the response is from a websocket
            if (!response.isWebSocket)
            {
                amx_Push(currentAmx, response.status);
            }

            amx_Push(currentAmx, response.id);

            amx_Exec(currentAmx, &amx_ret, amx_idx);
            amx_Release(currentAmx, amx_addr);

            break;
        }

        case Impl::E_CONTENT_TYPE::json:
        {
            cell id = -1;
            try
            {
                json::value *obj = new json::value;
                *obj = json::value::parse(utility::conversions::to_string_t(response.rawBody));
                id = JSON::Alloc(obj);
            }
            catch (std::exception e)
            {
                logprintf("ERROR: failed to parse response as JSON: '%s'", response.rawBody.c_str());
            }

            amx_Push(currentAmx, id);
            // signature is either
            // (Request:id, E_HTTP_STATUS:status, Node:node)
            // or:
            // (WebSocket:id, Node:node)
            // depending on whether the response is from a websocket
            if (!response.isWebSocket)
            {
                amx_Push(currentAmx, response.status);
            }
            amx_Push(currentAmx, response.id);

            amx_Exec(currentAmx, &amx_ret, amx_idx);

            JSON::Erase(id);
            break;
        }
        }
    }
}

int Natives::WebSocketClient(AMX *amx, cell *params)
{
    std::string address = amx_GetCppString(amx, params[1]);
    std::string callback = amx_GetCppString(amx, params[2]);
    return Impl::WebSocketClient(address, callback);
}

int Natives::WebSocketSend(AMX *amx, cell *params)
{
    int id = params[1];
    std::string data = amx_GetCppString(amx, params[2]);
    return Impl::WebSocketSend(id, data);
}

int Natives::JsonWebSocketClient(AMX *amx, cell *params)
{
    std::string address = amx_GetCppString(amx, params[1]);
    std::string callback = amx_GetCppString(amx, params[2]);
    return Impl::JsonWebSocketClient(address, callback);
}

int Natives::JsonWebSocketSend(AMX *amx, cell *params)
{
    int id = params[1];
    auto obj = JSON::Get(params[2]);
    return Impl::JsonWebSocketSend(id, obj);
}

// JSON implementation is directly in the Natives section unlike other API.
// this is purely to simplify things while working with JSON value types.

int Natives::JSON::Parse(AMX *amx, cell *params)
{
    std::string input = amx_GetCppString(amx, params[1]);
    cell *output;
    amx_GetAddr(amx, params[2], &output);

    web::json::value *obj = new web::json::value;

    try
    {
        *obj = web::json::value::parse(utility::conversions::to_string_t(input));
    }
    catch (std::exception &e)
    {
        logprintf("ERROR: JsonParse failed with: %s", e.what());
        return 1;
    }

    *output = Alloc(obj);

    return 0;
}

int Natives::JSON::Stringify(AMX *amx, cell *params)
{
    auto obj = Get(params[1], false);
    std::string s = utility::conversions::to_utf8string(obj.serialize());

    amx_SetCppString(amx, params[2], s, params[3]);

    return 0;
}

int Natives::JSON::NodeType(AMX *amx, cell *params)
{
    auto obj = Get(params[1], false);
    if (obj.is_null())
    {
        return web::json::value::Null;
    }
    return obj.type();
}

int Natives::JSON::Object(AMX *amx, cell *params)
{
    std::string key;
    std::vector<std::pair<utility::string_t, web::json::value>> fields;

    for (size_t i = 1; i <= params[0] / sizeof(cell); i++)
    {
        cell *addr = nullptr;
        amx_GetAddr(amx, params[i], &addr);

        if (addr == nullptr)
        {
            break;
        }

        if (i & 1)
        {
            int len = 0;
            amx_StrLen(addr, &len);
            if (len <= 0 || len > 512)
            {
                logprintf("ERROR: string length in Object out of bounds (%d)", len);
                return -1;
            }

            key = std::string(len, ' ');
            amx_GetString(&key[0], addr, 0, len + 1);
        }
        else
        {
            web::json::value obj = Get(*addr);
            if (obj == web::json::value::null())
            {
                logprintf("ERROR: value node %d was invalid", *addr);
                return -2;
            }
            fields.push_back(std::make_pair(utility::conversions::to_string_t(key), obj));
        }
    }

    web::json::value *obj = new web::json::value;
    *obj = web::json::value::object(fields);
    return Alloc(obj);
}

int Natives::JSON::Int(AMX *amx, cell *params)
{
    web::json::value *obj = new web::json::value;
    *obj = web::json::value::number(params[1]);
    return Alloc(obj);
}

int Natives::JSON::Float(AMX *amx, cell *params)
{
    web::json::value *obj = new web::json::value;
    *obj = web::json::value::number(amx_ctof(params[1]));
    return Alloc(obj);
}

int Natives::JSON::Bool(AMX *amx, cell *params)
{
    web::json::value *obj = new web::json::value;
    *obj = web::json::value::boolean(params[1]);
    return Alloc(obj);
}

int Natives::JSON::String(AMX *amx, cell *params)
{
    web::json::value *obj = new web::json::value;
    *obj = web::json::value::string(utility::conversions::to_string_t(amx_GetCppString(amx, params[1])));
    return Alloc(obj);
}

int Natives::JSON::Array(AMX *amx, cell *params)
{
    std::vector<web::json::value> fields;

    for (size_t i = 1; i <= params[0] / sizeof(cell); i++)
    {
        cell *addr = nullptr;
        amx_GetAddr(amx, params[i], &addr);

        if (addr == nullptr)
        {
            break;
        }

        auto obj = Get(*addr);
        if (obj == web::json::value::null())
        {
            logprintf("ERROR: value node %d was invalid", *addr);
            return -2;
        }
        fields.push_back(obj);
    }

    web::json::value *obj = new web::json::value;
    *obj = web::json::value::array(fields);
    return Alloc(obj);
}

int Natives::JSON::Append(AMX *amx, cell *params)
{
    web::json::value a = Get(params[1], false);
    web::json::value b = Get(params[2]);
    int result;

    if (a.is_object() && b.is_object())
    {
        web::json::value *c = new web::json::value;
        std::vector<std::pair<utility::string_t, web::json::value>> newObject;
        for (auto entry : a.as_object())
        {
            newObject.push_back(std::make_pair(entry.first, entry.second));
        }
        for (auto entry : b.as_object())
        {
            newObject.push_back(std::make_pair(entry.first, entry.second));
        }
        *c = web::json::value::object(newObject);
        result = Alloc(c);
    }
    else if (a.is_array() && b.is_array())
    {
        web::json::value *c = new web::json::value;
        std::vector<web::json::value> newArray;
        for (auto entry : a.as_array())
        {
            newArray.push_back(entry);
        }
        for (auto entry : b.as_array())
        {
            newArray.push_back(entry);
        }
        *c = web::json::value::array(newArray);
        result = Alloc(c);
    }
    else
    {
        return -1;
    }

    return result;
}

int Natives::JSON::SetObject(AMX *amx, cell *params)
{
    web::json::value *obj = GetPointer(params[1]);
    if (obj == nullptr)
    {
        return 1;
    }
    if (!obj->is_object())
    {
        return 2;
    }

    web::json::value value = Get(params[3]);
    if (value == web::json::value::null())
    {
        return 3;
    }

    utility::string_t key = utility::conversions::to_string_t(amx_GetCppString(amx, params[2]));
    obj->as_object()[key] = value;

    return 0;
}

int Natives::JSON::SetInt(AMX *amx, cell *params)
{
    web::json::value *obj = GetPointer(params[1]);
    if (obj == nullptr)
    {
        return 1;
    }
    if (!obj->is_object())
    {
        return 2;
    }

    utility::string_t key = utility::conversions::to_string_t(amx_GetCppString(amx, params[2]));
    obj->as_object()[key] = json::value::number(params[3]);

    return 0;
}

int Natives::JSON::SetFloat(AMX *amx, cell *params)
{
    web::json::value *obj = GetPointer(params[1]);
    if (obj == nullptr)
    {
        return 1;
    }
    if (!obj->is_object())
    {
        return 2;
    }

    utility::string_t key = utility::conversions::to_string_t(amx_GetCppString(amx, params[2]));
    obj->as_object()[key] = json::value::number(amx_ctof(params[3]));

    return 0;
}

int Natives::JSON::SetBool(AMX *amx, cell *params)
{
    web::json::value *obj = GetPointer(params[1]);
    if (obj == nullptr)
    {
        return 1;
    }
    if (!obj->is_object())
    {
        return 2;
    }

    utility::string_t key = utility::conversions::to_string_t(amx_GetCppString(amx, params[2]));
    obj->as_object()[key] = json::value::boolean(params[3]);

    return 0;
}

int Natives::JSON::SetString(AMX *amx, cell *params)
{
    web::json::value *obj = GetPointer(params[1]);
    if (obj == nullptr)
    {
        return 1;
    }
    if (!obj->is_object())
    {
        return 2;
    }

    utility::string_t key = utility::conversions::to_string_t(amx_GetCppString(amx, params[2]));
    obj->as_object()[key] = json::value::string(utility::conversions::to_string_t(amx_GetCppString(amx, params[3])));

    return 0;
}

int Natives::JSON::GetObjectAlt(AMX *amx, cell *params)
{
    web::json::value obj = Get(params[1]);
    if (obj == web::json::value::null())
    {
        return 1;
    }

    std::string key = amx_GetCppString(amx, params[2]);

    web::json::value *result = new web::json::value();
    try
    {
        *result = obj.as_object()[utility::conversions::to_string_t(key)];
    }
    catch (...)
    {
        return 2;
    }
    cell id = Alloc(result);

    cell *addr = nullptr;
    amx_GetAddr(amx, params[3], &addr);
    *addr = id;

    return 0;
}

int Natives::JSON::GetInt(AMX *amx, cell *params)
{
    web::json::value obj = Get(params[1], false);
    if (obj == web::json::value::null())
    {
        return 1;
    }
    std::string key = amx_GetCppString(amx, params[2]);

    web::json::value target;
    try
    {
        target = obj.as_object().at(utility::conversions::to_string_t(key));
    }
    catch (...)
    {
        return 2;
    }
    if (!target.is_integer())
    {
        return 3;
    }

    cell *addr = nullptr;
    amx_GetAddr(amx, params[3], &addr);
    *addr = target.as_integer();

    return 0;
}

int Natives::JSON::GetFloat(AMX *amx, cell *params)
{
    web::json::value obj = Get(params[1], false);
    if (obj == web::json::value::null())
    {
        return 1;
    }
    std::string key = amx_GetCppString(amx, params[2]);

    web::json::value target;
    try
    {
        target = obj.as_object().at(utility::conversions::to_string_t(key));
    }
    catch (...)
    {
        return 2;
    }
    if (!target.is_double())
    {
        return 3;
    }

    cell *addr = nullptr;
    amx_GetAddr(amx, params[3], &addr);
    float d = static_cast<float>(target.as_double());
    *addr = amx_ftoc(d);

    return 0;
}

int Natives::JSON::GetBool(AMX *amx, cell *params)
{
    web::json::value obj = Get(params[1], false);
    if (obj == web::json::value::null())
    {
        return 1;
    }
    std::string key = amx_GetCppString(amx, params[2]);

    web::json::value target;
    try
    {
        target = obj.as_object().at(utility::conversions::to_string_t(key));
    }
    catch (...)
    {
        return 2;
    }
    if (!target.is_boolean())
    {
        return 3;
    }

    cell *addr = nullptr;
    amx_GetAddr(amx, params[3], &addr);
    *addr = target.as_bool();

    return 0;
}

int Natives::JSON::GetString(AMX *amx, cell *params)
{
    web::json::value obj = Get(params[1], false);
    if (obj == web::json::value::null())
    {
        return 1;
    }
    std::string key = amx_GetCppString(amx, params[2]);

    web::json::value target;
    try
    {
        target = obj.as_object().at(utility::conversions::to_string_t(key));
    }
    catch (...)
    {
        return 2;
    }
    if (!target.is_string())
    {
        return 3;
    }

    return amx_SetCppString(amx, params[3], utility::conversions::to_utf8string(target.as_string()).c_str(), params[4]);
}

int Natives::JSON::GetArray(AMX *amx, cell *params)
{
    web::json::value obj = Get(params[1], false);
    if (obj == web::json::value::null())
    {
        return 1;
    }
    std::string key = amx_GetCppString(amx, params[2]);

    web::json::value *target = new web::json::value;
    try
    {
        *target = obj.as_object().at(utility::conversions::to_string_t(key));
    }
    catch (...)
    {
        return 2;
    }
    if (!target->is_array())
    {
        return 3;
    }

    cell *addr = nullptr;
    amx_GetAddr(amx, params[3], &addr);
    *addr = Alloc(target);

    return 0;
}

int Natives::JSON::ArrayLength(AMX *amx, cell *params)
{
    web::json::value obj = Get(params[1], false);
    if (!obj.is_array() || obj == web::json::value::null())
    {
        return 1;
    }

    cell *addr = nullptr;
    amx_GetAddr(amx, params[2], &addr);
    *addr = obj.as_array().size();

    return 0;
}

int Natives::JSON::ArrayObject(AMX *amx, cell *params)
{
    web::json::value obj = Get(params[1], false);
    if (!obj.is_array() || obj == web::json::value::null())
    {
        return 1;
    }

    web::json::value *result = new web::json::value();
    try
    {
        *result = obj.as_array().at(params[2]);
    }
    catch (...)
    {
        return 2;
    }
    cell id = Alloc(result);

    cell *addr = nullptr;
    amx_GetAddr(amx, params[3], &addr);
    *addr = id;

    return 0;
}

int Natives::JSON::GetNodeInt(AMX *amx, cell *params)
{
    web::json::value obj = Get(params[1]);
    if (!obj.is_integer() || obj == web::json::value::null())
    {
        return 1;
    }

    cell *addr = nullptr;
    amx_GetAddr(amx, params[2], &addr);
    *addr = obj.as_integer();

    return 0;
}

int Natives::JSON::GetNodeFloat(AMX *amx, cell *params)
{
    web::json::value obj = Get(params[1]);
    if (!obj.is_double() || obj == web::json::value::null())
    {
        return 1;
    }

    cell *addr = nullptr;
    amx_GetAddr(amx, params[2], &addr);
    float d = static_cast<float>(obj.as_double());
    *addr = amx_ftoc(d);

    return 0;
}

int Natives::JSON::GetNodeBool(AMX *amx, cell *params)
{
    web::json::value obj = Get(params[1]);
    if (!obj.is_boolean() || obj == web::json::value::null())
    {
        return 1;
    }

    cell *addr = nullptr;
    amx_GetAddr(amx, params[2], &addr);
    *addr = obj.as_bool();

    return 0;
}

int Natives::JSON::GetNodeString(AMX *amx, cell *params)
{
    web::json::value obj = Get(params[1]);
    if (!obj.is_string() || obj == web::json::value::null())
    {
        return 1;
    }

    return amx_SetCppString(amx, params[2], utility::conversions::to_utf8string(obj.as_string()).c_str(), params[3]);
}

int Natives::JSON::ToggleGC(AMX *amx, cell *params)
{
    auto n = nodeTable.find(params[1]);
    if (n == nodeTable.end())
    {
        return 1;
    }

    n->second.gc = params[2];

    return 0;
}

int Natives::JSON::Cleanup(AMX *amx, cell *params)
{
    auto n = nodeTable.find(params[1]);
    if (n == nodeTable.end())
    {
        if (!params[2])
        {
            logprintf("ERROR: attempt to cleanup node from invalid ID %d", params[1]);
        }
        return 1;
    }

    if (!n->second.gc && params[2])
    {
        return 2;
    }

    Erase(params[1]);

    return 0;
}

cell Natives::JSON::Alloc(web::json::value *item)
{
    int id = jsonPoolCounter++;
    nodeTable[id] = {item, true};
    return id;
}

web::json::value Natives::JSON::Get(int id, bool gc)
{
    auto n = nodeTable.find(id);
    if (n == nodeTable.end())
    {
        return web::json::value::null();
    }

    // deref the node into a local copy for returning
    web::json::value copy = *(n->second.value);
    if (gc && n->second.gc)
    {
        // if gc, then delete the heap copy
        Erase(id);
    }
    // and return the copy
    return copy;
}

web::json::value *Natives::JSON::GetPointer(int id)
{
    auto n = nodeTable.find(id);
    if (n == nodeTable.end())
    {
        return nullptr;
    }
    return n->second.value;
}

void Natives::JSON::Erase(int id)
{
    auto n = nodeTable.find(id);
    if (n == nodeTable.end())
    {
        return;
    }
    delete n->second.value;
    nodeTable.erase(id);
}
