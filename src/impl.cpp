/*
# impl.cpp

As with the header, this is the actual implementation of the plugin's
functionality with no AMX specific code or includes.

Including `common.hpp` for access to `logprintf` is useful for debugging but for
production debug logging, it's best to use a dedicated logging library such as
log-core by maddinat0r.
*/

#include "impl.hpp"

std::unordered_map<int, http_client*> Impl::clientsTable;
int Impl::clientsTableCounter = 0;

std::unordered_map<int, std::vector<std::pair<std::string, std::string>>> Impl::headersTable;
int Impl::headersTableCounter = 0;

std::stack<Impl::CallbackTask> Impl::message_stack;
std::mutex Impl::message_stack_mutex;

int Impl::RestfulClient(std::string endpoint, std::vector<std::string> headers)
{
    int id = clientsTableCounter++;
    http_client* client = new http_client(utility::conversions::to_string_t(endpoint));
    clientsTable[id] = client;
    return id;
}

int Impl::RestfulGetData(int id, std::string path, std::string callback, std::vector<std::string> headers)
{
    http_client* client = nullptr;
    try {
        client = clientsTable[id];
    } catch (std::exception e) {
        return 1;
    }
    if (client == nullptr) {
        return 2;
    }

    http_request request(methods::GET);
    request.headers().add(U("Content-Type"), U("text/plain"));
    request.set_request_uri(utility::conversions::to_string_t(path));

    client->request(request).then([=](http_response response) {
        status_code status = response.status_code();

        auto a = response.extract_json().get();

        logprintf("response %s", utility::conversions::to_utf8string(a.serialize()).c_str());

        message_stack_mutex.lock();
        message_stack.push({ "hello" });
        message_stack_mutex.unlock();
        return;
    });

    return 0;
}

int Impl::RestfulPostData(int id, std::string endpoint, std::string callback, char* data, std::vector<std::string> headers)
{
    return 0;
}

int Impl::RestfulGetJSON(int id, std::string endpoint, std::string callback, std::vector<std::string> headers)
{
    return 0;
}

int Impl::RestfulPostJSON(int id, std::string endpoint, std::string callback, web::json::object json, std::vector<std::string> headers)
{
    return 0;
}

int Impl::RestfulHeaders(std::vector<std::pair<std::string, std::string>> headers)
{
    int id = headersTableCounter++;
    headersTable[id] = headers;
    return id;
}

int Impl::RestfulHeadersCleanup(int id)
{
    headersTable.erase(id);
    return 0;
}

std::vector<Impl::CallbackTask> Impl::gatherTasks()
{
    std::vector<CallbackTask> tasks;

    // if we can't lock the mutex, don't block, just return and try next tick
    if (message_stack_mutex.try_lock()) {
        CallbackTask cbt;
        while (!message_stack.empty()) {
            cbt = message_stack.top();

            logprintf("received message");

            tasks.push_back(cbt);
            message_stack.pop();
        }
        message_stack_mutex.unlock();
    }

    return tasks;
}
