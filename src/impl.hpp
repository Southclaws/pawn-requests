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
#include <thread>

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
enum E_HTTP_METHOD {
    HTTP_METHOD_GET = 0,
    HTTP_METHOD_HEAD,
    HTTP_METHOD_POST,
    HTTP_METHOD_PUT,
    HTTP_METHOD_DELETE,
    HTTP_METHOD_CONNECT,
    HTTP_METHOD_OPTIONS,
    HTTP_METHOD_TRACE,
    HTTP_METHOD_PATCH
};
enum E_CONTENT_TYPE {
	empty,
    string,
    json
};
struct RequestData {
    int id;
    std::string callback;
    std::string path;
    E_HTTP_METHOD method;
	E_CONTENT_TYPE requestType;
	E_CONTENT_TYPE responseType;
	int headers;
	std::string bodyString;
	web::json::value bodyJson;
};
struct ResponseData {
    int id;
    std::string callback;
    int status;
    E_CONTENT_TYPE responseType;
    std::string rawBody;
};

int RequestsClient(std::string endpoint, int headers);
int RequestHeaders(std::vector<std::pair<std::string, std::string>> headers);
int RequestText(int id, std::string path, E_HTTP_METHOD method, E_CONTENT_TYPE responseType, std::string callback, char* data, int headers);
int RequestJSON(int id, std::string path, E_HTTP_METHOD method, E_CONTENT_TYPE responseType, std::string callback, web::json::value json, int headers);

struct ClientData {
    http_client* client;
    std::vector<std::pair<std::string, std::string>> headers;
};
int headersCleanup(int id);
int doRequest(int id, RequestData data);
void doRequestThread(ClientData cd, RequestData requestData);
web::http::method methodName(E_HTTP_METHOD id);

extern int requestCounter;
extern std::stack<ResponseData> taskStack;
extern std::mutex taskStackLock;
// gatherTasks is called by the Natives to get a list of callbacks to call
std::vector<ResponseData> gatherResponses();

extern std::unordered_map<int, ClientData> clientsTable;
extern int clientsTableCounter;

extern std::unordered_map<int, std::vector<std::pair<std::string, std::string>>> headersTable;
extern int headersTableCounter;
};

#endif
