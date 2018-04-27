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

#ifndef REQUESTS_IMPL_H
#define REQUESTS_IMPL_H

namespace Impl {
int RestfulClient(std::string endpoint, int headers);
int RestfulHeaders(std::vector<std::pair<std::string, std::string>> headers);
int RestfulRequestText(int id, std::string path, int method, int responseType, std::string callback, char* data, int headers);
int RestfulRequestJSON(int id, std::string path, int method, int responseType, std::string callback, web::json::value json, int headers);

int RestfulHeadersCleanup(int id);
int doRequest(int id, std::string path, std::string callback, RequestData data);

extern int requestCounter;

enum E_TASK_TYPE {
    string,
    json
};
struct RequestData {
    // shared fields
    int id;
    std::string callback;

    // response fields
    E_TASK_TYPE type;
    int status;
    std::string string;
    json::value json;
};
extern std::stack<RequestData> taskStack;
extern std::mutex taskStackLock;
// gatherTasks is called by the Natives to get a list of callbacks to call
std::vector<RequestData> gatherTasks();

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
