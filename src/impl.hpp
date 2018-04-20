/*
# impl.hpp

From here on, it's just regular C++ code, no AMX/Pawn/SA:MP stuff. This header
is for declaring implementation functions for the plugin's core functionality.
*/

#include <stack>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
#include <cpprest/json.h>

#include "common.hpp"

using namespace utility; // Common utilities like string conversions
using namespace web; // Common features like URIs.
using namespace web::http; // Common HTTP functionality
using namespace web::http::client; // HTTP client features
using namespace concurrency::streams; // Asynchronous streams

#ifndef RESTFUL_IMPL_H
#define RESTFUL_IMPL_H

namespace Impl {
int RestfulClient(std::string endpoint, std::vector<std::string> headers);
int RestfulGetData(int id, std::string endpoint, std::string callback, std::vector<std::string> headers);
int RestfulPostData(int id, std::string endpoint, std::string callback, char* data, std::vector<std::string> headers);
int RestfulGetJSON(int id, std::string endpoint, std::string callback, std::vector<std::string> headers);
int RestfulPostJSON(int id, std::string endpoint, std::string callback, web::json::object json, std::vector<std::string> headers);
int RestfulHeaders(std::vector<std::pair<std::string, std::string>> headers);
int RestfulHeadersCleanup(int id);

struct CallbackTask {
    std::string callback;
};
extern std::stack<CallbackTask> message_stack;
extern std::mutex message_stack_mutex;
// gatherTasks is called by the Natives to get a list of callbacks to call
std::vector<CallbackTask> gatherTasks();

extern std::unordered_map<int, http_client*> clientsTable;
extern int clientsTableCounter;

extern std::unordered_map<int, std::vector<std::pair<std::string, std::string>>> headersTable;
extern int headersTableCounter;
};

#endif
