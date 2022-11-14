#include "impl.hpp"

int Impl::requestCounter = 0;

std::stack<Impl::ResponseData> Impl::responseQueue;
std::mutex Impl::responseQueueLock;

std::unordered_map<int, Impl::ClientData> Impl::clientsTable;
int Impl::clientsTableCounter = 0;

std::unordered_map<int, std::vector<std::pair<std::string, std::string>>> Impl::headersTable;
int Impl::headersTableCounter = 0;

std::unordered_map<int, Impl::WebSocketClientData> Impl::websocketClientsTable;
int Impl::websocketClientsTableCounter = 0;

int Impl::RequestsClient(std::string endpoint, int headers)
{
    int id = clientsTableCounter++;
    try
    {
        http_client_config config;
#ifndef _WIN32
        // TODO: in the future allow the user to specify options to configure
        // TODO: the client with
        config.set_ssl_context_callback([](boost::asio::ssl::context &ctx)
                                        { ctx.load_verify_file("/etc/ssl/certs/ca-certificates.crt"); });
#endif
        http_client *client = new http_client(utility::conversions::to_string_t(endpoint), config);
        clientsTable[id] = {client, headersTable[headers]};
    }
    catch (std::exception &e)
    {
        logprintf("ERROR: Failed to create new HTTP client: %s", e.what());
        id = -1;
    }
    return id;
}

int Impl::RequestHeaders(std::vector<std::pair<std::string, std::string>> headers)
{
    int id = headersTableCounter++;
    headersTable[id] = headers;
    return id;
}

int Impl::Request(AMX *amx, int id, std::string path, E_HTTP_METHOD method, std::string callback, char *data, int headers)
{
    RequestData requestData;
    requestData.amx = amx;
    requestData.id = requestCounter;
    requestData.callback = callback;
    requestData.path = path;
    requestData.method = method;
    requestData.requestType = E_CONTENT_TYPE::string;
    requestData.headers = headers;
    requestData.bodyString = data;

    int ret = doRequest(id, requestData);
    if (ret < 0)
    {
        return ret;
    }
    return requestCounter++;
}

int Impl::RequestJSON(AMX *amx, int id, std::string path, E_HTTP_METHOD method, std::string callback, web::json::value json, int headers)
{
    RequestData requestData;
    requestData.amx = amx;
    requestData.id = requestCounter;
    requestData.callback = callback;
    requestData.path = path;
    requestData.method = method;
    requestData.requestType = E_CONTENT_TYPE::json;
    requestData.headers = headers;
    requestData.bodyJson = json;

    int ret = doRequest(id, requestData);
    if (ret < 0)
    {
        return ret;
    }
    return requestCounter++;
}

int Impl::headersCleanup(int id)
{
    headersTable.erase(id);
    return 0;
}

int Impl::doRequest(int id, RequestData requestData)
{
    if (clientsTable.find(id) == clientsTable.end())
    {
        logprintf("ERROR: invalid client ID %d used", id);
        return -1;
    }
    ClientData cd = clientsTable[id];

    try
    {
        std::thread t(doRequestWithClient, cd, requestData);
        t.detach();
    }
    catch (std::exception e)
    {
        logprintf("ERROR: failed to dispatch request thread: '%s'", e.what());
        return -2;
    }

    return 0;
}

void Impl::doRequestWithClient(ClientData cd, RequestData requestData)
{
    ResponseData responseData;
    responseData.amx = requestData.amx;
    responseData.id = requestData.id;
    responseData.callback = requestData.callback;
    responseData.responseType = E_CONTENT_TYPE::empty;

    try
    {
        doRequestSync(cd, requestData, responseData);
    }
    catch (http::http_exception e)
    {
        logprintf("ERROR: HTTP error %s", e.what());
        responseData.callback = "OnRequestFailure";
        responseData.rawBody = e.what();
        responseData.status = 1;
    }
    catch (std::exception e)
    {
        logprintf("ERROR: General error %s", e.what());
        responseData.callback = "OnRequestFailure";
        responseData.rawBody = e.what();
        responseData.status = 2;
    }
    catch (...)
    {
        try
        {
            auto eptr = std::current_exception();
            if (eptr)
            {
                std::rethrow_exception(eptr);
            }
        }
        catch (const std::exception &e)
        {
            logprintf("ERROR: Unknown error %s", e.what());
            responseData.callback = "OnRequestFailure";
            responseData.rawBody = e.what();
            responseData.status = 3;
        }
    }

    responseQueueLock.lock();
    responseQueue.push(responseData);
    responseQueueLock.unlock();
}

void Impl::doRequestSync(ClientData cd, RequestData requestData, ResponseData &responseData)
{
    http_request request(methodName(requestData.method));
    for (auto h : cd.headers)
    {
        request.headers().add(
            utility::conversions::to_string_t(h.first),
            utility::conversions::to_string_t(h.second));
    }
    for (auto h : headersTable[requestData.headers])
    {
        request.headers().add(
            utility::conversions::to_string_t(h.first),
            utility::conversions::to_string_t(h.second));
    }
    request.set_request_uri(utility::conversions::to_string_t(requestData.path));

    switch (requestData.requestType)
    {
    case E_CONTENT_TYPE::json:
    {
        if (!requestData.bodyJson.is_null())
        {
            request.set_body(requestData.bodyJson);
        }
        request.headers().set_content_type(U("application/json"));
        break;
    }
    case E_CONTENT_TYPE::string:
    {
        request.set_body(requestData.bodyString);
        break;
    }
    }

    http_response response = cd.client->request(request).get();
    std::string body = response.extract_utf8string().get();

    responseData.status = response.status_code();
    responseData.rawBody = body;
    responseData.responseType = requestData.requestType;
}

web::http::method Impl::methodName(E_HTTP_METHOD id)
{
    switch (id)
    {
    case E_HTTP_METHOD::HTTP_METHOD_GET:
        return utility::conversions::to_string_t("GET");
    case E_HTTP_METHOD::HTTP_METHOD_HEAD:
        return utility::conversions::to_string_t("HEAD");
    case E_HTTP_METHOD::HTTP_METHOD_POST:
        return utility::conversions::to_string_t("POST");
    case E_HTTP_METHOD::HTTP_METHOD_PUT:
        return utility::conversions::to_string_t("PUT");
    case E_HTTP_METHOD::HTTP_METHOD_DELETE:
        return utility::conversions::to_string_t("DELETE");
    case E_HTTP_METHOD::HTTP_METHOD_CONNECT:
        return utility::conversions::to_string_t("CONNECT");
    case E_HTTP_METHOD::HTTP_METHOD_OPTIONS:
        return utility::conversions::to_string_t("OPTIONS");
    case E_HTTP_METHOD::HTTP_METHOD_TRACE:
        return utility::conversions::to_string_t("TRACE");
    case E_HTTP_METHOD::HTTP_METHOD_PATCH:
        return utility::conversions::to_string_t("PATCH");
    }
    throw std::invalid_argument("HTTP method not found in enumerator");
}

std::vector<Impl::ResponseData> Impl::gatherResponses()
{
    std::vector<ResponseData> tasks;

    // if we can't lock the mutex, don't block, just return and try next tick
    if (responseQueueLock.try_lock())
    {
        ResponseData response;
        while (!responseQueue.empty())
        {
            response = responseQueue.top();
            tasks.push_back(response);
            responseQueue.pop();
        }
        responseQueueLock.unlock();
    }

    return tasks;
}

int Impl::WebSocketClient(std::string address, std::string callback)
{
    int id = websocketClientsTableCounter++;

    websocket_client_config wcc;
#ifndef _WIN32
    wcc.set_ssl_context_callback([](boost::asio::ssl::context &ctx)
                                 { ctx.load_verify_file("/etc/ssl/certs/ca-certificates.crt"); });
#endif
    websocket_callback_client *client = new websocket_callback_client(wcc);
    if (client == nullptr)
    {
        return -1;
    }

    WebSocketClientData wsc = {id, client, address, callback, false};
    websocketClientsTable[id] = wsc;
    try
    {
        startWebSocketListener(wsc);
    }
    catch (std::exception &e)
    {
        logprintf("ERROR: WebSocketClient failed: %s", e.what());
        return -1;
    }

    return id;
}

int Impl::WebSocketSend(int id, std::string data)
{
    WebSocketClientData wsc;
    try
    {
        wsc = websocketClientsTable[id];
    }
    catch (std::exception e)
    {
        return -1;
    }

    websocket_outgoing_message msg;
    msg.set_utf8_message(data);
    wsc.client->send(msg);

    return 0;
}

int Impl::JsonWebSocketClient(std::string address, std::string callback)
{
    int id = websocketClientsTableCounter++;

    websocket_callback_client *client = new websocket_callback_client();
    if (client == nullptr)
    {
        return -1;
    }

    WebSocketClientData wsc = {id, client, address, callback, true};
    websocketClientsTable[id] = wsc;
    try
    {
        startWebSocketListener(wsc);
    }
    catch (std::exception &e)
    {
        logprintf("ERROR: JsonWebSocketClient failed: %s", e.what());
        return -1;
    }

    return id;
}

int Impl::JsonWebSocketSend(int id, web::json::value json)
{
    WebSocketClientData wsc;
    try
    {
        wsc = websocketClientsTable[id];
    }
    catch (std::exception e)
    {
        return -1;
    }

    websocket_outgoing_message msg;
    msg.set_utf8_message(utility::conversions::to_utf8string(json.serialize()));
    wsc.client->send(msg);

    return 0;
}

void Impl::startWebSocketListener(WebSocketClientData wsc)
{
    wsc.client->set_message_handler([wsc](const websocket_incoming_message &msg) -> void
                                    {
        std::string raw = msg.extract_string().get();

        ResponseData responseData;
        responseData.id = wsc.id;
        responseData.callback = wsc.callback;
        responseData.rawBody = raw;
        responseData.responseType = wsc.isJson ? E_CONTENT_TYPE::json : E_CONTENT_TYPE::string;
        responseData.isWebSocket = true;

        responseQueueLock.lock();
        responseQueue.push(responseData);
        responseQueueLock.unlock(); });

    wsc.client->connect(utility::conversions::to_string_t(wsc.address)).wait();
}
