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

int Impl::RequestsClient(std::string endpoint, int headers)
{
    int id = clientsTableCounter++;
    http_client* client = new http_client(utility::conversions::to_string_t(endpoint));
    clientsTable[id] = { client, headersTable[headers] };
    return id;
}

int Impl::RequestHeaders(std::vector<std::pair<std::string, std::string>> headers)
{
    int id = headersTableCounter++;
    headersTable[id] = headers;
    return id;
}

int Impl::RequestText(int id, std::string path, E_HTTP_METHOD method, E_CONTENT_TYPE responseType, std::string callback, char* data, int headers)
{
    RequestData requestData;
    requestData.id = requestCounter;
    requestData.callback = callback;
    requestData.path = path;
    requestData.method = method;
    requestData.requestType = E_CONTENT_TYPE::string;
    requestData.responseType = responseType;
    requestData.headers = headers;
    requestData.bodyString = data;

    int ret = doRequest(id, requestData);
    if (ret < 0) {
        return ret;
    }
    return requestCounter++;
}

int Impl::RequestJSON(int id, std::string path, E_HTTP_METHOD method, E_CONTENT_TYPE responseType, std::string callback, web::json::value json, int headers)
{
    RequestData requestData;
    requestData.id = requestCounter;
    requestData.callback = callback;
    requestData.path = path;
    requestData.method = method;
    requestData.requestType = E_CONTENT_TYPE::json;
    requestData.responseType = responseType;
    requestData.headers = headers;
    requestData.bodyJson = json;

    int ret = doRequest(id, requestData);
    if (ret < 0) {
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
    ClientData cd;
    try {
        cd = clientsTable[id];
    } catch (std::exception e) {
        return -1;
    }

    // note: synchronous for dev/tests, will thread for release
    // std::thread t(doRequestThread, cd, requestData);
    // t.join();
    try {
        doRequestWithClient(cd, requestData);
    } catch (std::exception e) {
        logprintf("exception: '%s'", e.what());
		return 1;
    }

    return 0;
}

void Impl::doRequestWithClient(ClientData cd, RequestData requestData)
{
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

    switch (requestData.requestType) {
    case E_CONTENT_TYPE::json: {
        request.set_body(requestData.bodyJson);
        break;
    }
    case E_CONTENT_TYPE::string: {
        request.set_body(requestData.bodyString);
        break;
    }
    }

	logprintf("performing request %d", requestData.id);
    http_response response = cd.client->request(request).get();
    std::string body = response.extract_utf8string().get();
	logprintf("got body, len %d", body.length());

    ResponseData responseData;
    responseData.id = requestData.id;
    responseData.callback = requestData.callback;
    responseData.status = response.status_code();
    responseData.responseType = requestData.responseType;
    responseData.rawBody = body;

    taskStackLock.lock();
    taskStack.push(responseData);
    taskStackLock.unlock();
}

web::http::method Impl::methodName(E_HTTP_METHOD id)
{
    switch (id) {
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
    if (taskStackLock.try_lock()) {
        ResponseData response;
        while (!taskStack.empty()) {
            response = taskStack.top();
            tasks.push_back(response);
            taskStack.pop();
        }
        taskStackLock.unlock();
    }

    return tasks;
}
