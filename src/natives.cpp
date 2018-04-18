/*
# natives.cpp

This source file contains the bridge between natives and implementations. I
prefer to keep the actual implementation separate. The implementation contains
no instances of `cell` or `AMX*` and is purely C++ and external library code.
The code here acts as the translation between AMX data types and native types.
*/

#include "natives.hpp"
// #include "plugin-natives\NativeFunc.hpp"

std::unordered_map<int, web::json::value> Natives::JSON::jsonPool;
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

int Natives::JSON::JsonObject(AMX* amx, cell* params)
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

		if (addr == 0) {
			break;
		}

		if (arg & 1) {
			int id = *addr;
			auto obj = JsonGet(id);
			logprintf("- reading object id %d", id);
			fields.push_back(std::make_pair(utility::conversions::to_string_t(key), obj));
		} else {
			if (*addr == 0) {
				break;
			}

			int len = 0;
			amx_StrLen(addr, &len);
			if (len <= 0 || len > 512) {
				logprintf("error: string length in JsonObject out of bounds (%d)", len);
				return 0;
			}

			key = std::string(len, ' ');
			amx_GetString(&key[0], addr, 0, len + 1);
		}

		arg++;
	}

	web::json::value obj = web::json::value::object(fields);
	int id = JsonAlloc(obj);

	std::string s = utility::conversions::to_utf8string(obj.serialize());
	logprintf("JSON: '%s'", s.c_str());

    return id;
}

int Natives::JSON::JsonString(AMX* amx, cell* params)
{
	web::json::value obj = web::json::value::string(utility::conversions::to_string_t(amx_GetCppString(amx, params[1])));
	int id = JsonAlloc(obj);
	logprintf("JsonString: %d", id);
	return id;
}

int Natives::JSON::JsonNumber(AMX* amx, cell* params)
{
	web::json::value obj = web::json::value::number(params[1]);
	int id = JsonAlloc(obj);
	logprintf("JsonNumber: %d", id);
	return id;
}

int Natives::JSON::JsonArray(AMX* amx, cell* params)
{
    return 0;
}

int Natives::JSON::JsonStringify(AMX* amx, cell* params)
{
	auto obj = JsonGet(params[1]);
	std::string s = utility::conversions::to_utf8string(obj.serialize());

	amx_SetCppString(amx, params[2], s, params[3]);

	return 0;
}

int Natives::JSON::JsonAlloc(web::json::value item)
{
	int id = jsonPoolCounter++;
	jsonPool[id] = item;
	return id;
}

web::json::value Natives::JSON::JsonGet(int id) {
	if (id < 0 || id > jsonPoolCounter) {
		logprintf("error: id %d out of range %d", id, jsonPoolCounter);
		return web::json::value();
	}
	return jsonPool[id];
}

