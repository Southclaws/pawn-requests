#include <stack>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <boost/asio/ssl.hpp>

#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <cpprest/ws_client.h>

#include "common.hpp"
#include <amx/amx2.h>

using namespace utility;                 // Common utilities like string conversions
using namespace web;                     // Common features like URIs.
using namespace web::http;               // Common HTTP functionality
using namespace web::http::client;       // HTTP client features
using namespace web::websockets::client; // Websocket client features
using namespace concurrency::streams;    // Asynchronous streams

#ifndef REQUESTS_IMPL_H
#define REQUESTS_IMPL_H

namespace Impl
{
    enum E_HTTP_METHOD
    {
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
    enum E_CONTENT_TYPE
    {
        empty,
        string,
        json
    };
    struct RequestData
    {
        AMX *amx;
        int id;
        std::string callback;
        std::string path;
        E_HTTP_METHOD method;
        E_CONTENT_TYPE requestType;
        int headers;
        std::string bodyString;
        web::json::value bodyJson;
    };
    struct ResponseData
    {
        AMX *amx;
        int id;
        std::string callback;
        int status;
        E_CONTENT_TYPE responseType;
        std::string rawBody;
        bool isWebSocket = false;
    };

    int RequestsClient(std::string endpoint, int headers);
    int RequestHeaders(std::vector<std::pair<std::string, std::string>> headers);
    int Request(AMX *amx, int id, std::string path, E_HTTP_METHOD method, std::string callback, char *data, int headers);
    int RequestJSON(AMX *amx, int id, std::string path, E_HTTP_METHOD method, std::string callback, web::json::value json, int headers);

    int WebSocketClient(std::string address, std::string callback);
    int WebSocketSend(int id, std::string data);
    int JsonWebSocketClient(std::string address, std::string callback);
    int JsonWebSocketSend(int id, web::json::value json);

    struct ClientData
    {
        http_client *client;
        std::vector<std::pair<std::string, std::string>> headers;
    };
    struct WebSocketClientData
    {
        int id;
        websocket_callback_client *client;
        std::string address;
        std::string callback;
        bool isJson;
    };
    int headersCleanup(int id);
    int doRequest(int id, RequestData data);
    void doRequestWithClient(ClientData cd, RequestData requestData);
    void doRequestSync(ClientData cd, RequestData requestData, ResponseData &responseData);
    web::http::method methodName(E_HTTP_METHOD id);
    void startWebSocketListener(WebSocketClientData wsc);

    extern int requestCounter;
    extern std::stack<ResponseData> responseQueue;
    extern std::mutex responseQueueLock;
    std::vector<ResponseData> gatherResponses();

    extern std::unordered_map<int, ClientData> clientsTable;
    extern int clientsTableCounter;

    extern std::unordered_map<int, std::vector<std::pair<std::string, std::string>>> headersTable;
    extern int headersTableCounter;

    extern std::unordered_map<int, WebSocketClientData> websocketClientsTable;
    extern int websocketClientsTableCounter;
};

#endif
