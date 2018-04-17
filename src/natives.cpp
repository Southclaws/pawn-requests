/*
# natives.cpp

This source file contains the bridge between natives and implementations. I
prefer to keep the actual implementation separate. The implementation contains
no instances of `cell` or `AMX*` and is purely C++ and external library code.
The code here acts as the translation between AMX data types and native types.
*/

#include "natives.hpp"
// #include "plugin-natives\NativeFunc.hpp"

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

int Natives::JsonObject(AMX* amx, cell* params)
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
			fields.push_back(std::make_pair(utility::conversions::to_string_t(key), *addr));
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
	std::string s = utility::conversions::to_utf8string(obj.serialize());
	logprintf("JSON: '%s'", s.c_str());

    return 5;
}

int Natives::JsonString(AMX* amx, cell* params)
{
    return 0;
}

int Natives::JsonNumber(AMX* amx, cell* params)
{
    return 0;
}

int Natives::JsonArray(AMX* amx, cell* params)
{
    return 0;
}

int Natives::JsonStringify(AMX* amx, cell* params)
{
    return 0;
}
