/*
# impl.cpp

As with the header, this is the actual implementation of the plugin's
functionality with no AMX specific code or includes.

Including `common.hpp` for access to `logprintf` is useful for debugging but for
production debug logging, it's best to use a dedicated logging library such as
log-core by maddinat0r.
*/

#include "impl.hpp"

int Impl::requestCounter = 0;

std::stack<Impl::ResponseData> Impl::taskStack;
std::mutex Impl::taskStackLock;

std::unordered_map<int, Impl::ClientData> Impl::clientsTable;
int Impl::clientsTableCounter = 0;

std::unordered_map<int, std::vector<std::pair<std::string, std::string>>> Impl::headersTable;
int Impl::headersTableCounter = 0;

int Impl::RestfulClient(std::string endpoint, int headers)
{
    int id = clientsTableCounter++;
    http_client* client = new http_client(utility::conversions::to_string_t(endpoint));
    clientsTable[id] = { client, headersTable[headers] };
    return id;
}

int Impl::RestfulRequestText(int id, std::string path, E_HTTP_METHOD method, E_RESPONSE_TYPE responseType, std::string callback, char* data, int headers)
{
    RequestData requestData;
    requestData.id = requestCounter;
    requestData.callback = callback;
    requestData.path = path;
    requestData.method = method;
    requestData.responseType = responseType;
    requestData.headers = headers;

    int ret = doRequest(id, requestData);
    if (ret < 0) {
        return ret;
    }
    return requestCounter++;
}

int Impl::RestfulRequestJSON(int id, std::string path, E_HTTP_METHOD method, E_RESPONSE_TYPE responseType, std::string callback, web::json::value json, int headers)
{
    return requestCounter++;
}

int Impl::doRequest(int id, RequestData requestData)
{
    ClientData cd;
    try {
        cd = clientsTable[id];
    } catch (std::exception e) {
        return -1;
    }

    http_request request(methodName(requestData.method));
    for (auto h : cd.headers) {
        request.headers().add(
            utility::conversions::to_string_t(h.first),
            utility::conversions::to_string_t(h.second));
    }
    for (auto h : headersTable[requestData.headers]) {
        request.headers().add(
            utility::conversions::to_string_t(h.first),
            utility::conversions::to_string_t(h.second));
    }
    request.set_request_uri(utility::conversions::to_string_t(requestData.path));

    cd.client->request(request).then([=](http_response response) {
        ResponseData responseData;

        responseData.id = requestData.id;
        responseData.callback = requestData.callback;
        responseData.status = response.status_code();
        responseData.responseType = requestData.responseType;
        responseData.rawBody = response.extract_utf8string().get();

        taskStackLock.lock();
        taskStack.push(responseData);
        taskStackLock.unlock();
    });

    return 0;
}

web::http::method Impl::methodName(E_HTTP_METHOD id)
{
    switch (id) {
    case E_HTTP_METHOD::HTTP_METHOD_GET:
        return web::http::methods::GET;
    case E_HTTP_METHOD::HTTP_METHOD_HEAD:
        return web::http::methods::HEAD;
    case E_HTTP_METHOD::HTTP_METHOD_POST:
        return web::http::methods::POST;
    case E_HTTP_METHOD::HTTP_METHOD_PUT:
        return web::http::methods::PUT;
    case E_HTTP_METHOD::HTTP_METHOD_DELETE:
        return web::http::methods::DEL;
    case E_HTTP_METHOD::HTTP_METHOD_CONNECT:
        return web::http::methods::CONNECT;
    case E_HTTP_METHOD::HTTP_METHOD_OPTIONS:
        return web::http::methods::OPTIONS;
    case E_HTTP_METHOD::HTTP_METHOD_TRACE:
        return web::http::methods::TRCE;
    case E_HTTP_METHOD::HTTP_METHOD_PATCH:
        return web::http::methods::PATCH;
    }
    return "";
}

int Impl::RestfulHeaders(std::vector<std::pair<std::string, std::string>> headers)
{
    int id = headersTableCounter++;
    headersTable[id] = headers;
    return id;
}

int Impl::headersCleanup(int id)
{
    headersTable.erase(id);
    return 0;
}

std::vector<Impl::ResponseData> Impl::gatherTasks()
{
    std::vector<ResponseData> tasks;

    // if we can't lock the mutex, don't block, just return and try next tick
    if (taskStackLock.try_lock()) {
        ResponseData cbt;
        while (!taskStack.empty()) {
            cbt = taskStack.top();
            tasks.push_back(cbt);
            taskStack.pop();
        }
        taskStackLock.unlock();
    }

    return tasks;
}
