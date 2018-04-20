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
int RestfulClient(std::string endpoint, int headers);
int RestfulGetData(int id, std::string endpoint, std::string callback, int headers);
int RestfulPostData(int id, std::string endpoint, std::string callback, char* data, int headers);
int RestfulGetJSON(int id, std::string endpoint, std::string callback, int headers);
int RestfulPostJSON(int id, std::string endpoint, std::string callback, web::json::object json, int headers);
int RestfulHeaders(std::vector<std::pair<std::string, std::string>> headers);
int RestfulHeadersCleanup(int id);

extern int requestCounter;

enum E_TASK_TYPE {
    string,
    json
};
struct CallbackTask {
    int id;
    std::string callback;
    int status;
    E_TASK_TYPE type;
    std::string string;
    json::value json;
};
extern std::stack<CallbackTask> taskStack;
extern std::mutex taskStackLock;
// gatherTasks is called by the Natives to get a list of callbacks to call
std::vector<CallbackTask> gatherTasks();

struct ClientData {
	http_client* client;
	std::vector<std::pair<std::string, std::string>> headers;
};
extern std::unordered_map<int, ClientData> clientsTable;
extern int clientsTableCounter;

extern std::unordered_map<int, std::vector<std::pair<std::string, std::string>>> headersTable;
extern int headersTableCounter;
};

#endif
