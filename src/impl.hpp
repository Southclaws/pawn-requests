/*
# impl.hpp

From here on, it's just regular C++ code, no AMX/Pawn/SA:MP stuff. This header
is for declaring implementation functions for the plugin's core functionality.
*/

#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
#include <cpprest/json.h>

#ifndef RESTFUL_IMPL_H
#define RESTFUL_IMPL_H

namespace Impl {
int RestfulGetData(std::string endpoint, std::string callback, std::vector<std::string> headers);
int RestfulPostData(std::string endpoint, std::string callback, char* data, std::vector<std::string> headers);
int RestfulGetJSON(std::string endpoint, std::string callback, std::vector<std::string> headers);
int RestfulPostJSON(std::string endpoint, std::string callback, web::json::object json, std::vector<std::string> headers);
int RestfulHeaders(std::string ...);
};

#endif
