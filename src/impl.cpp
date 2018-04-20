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

std::stack<Impl::CallbackTask> Impl::taskStack;
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

int Impl::RestfulGetData(int id, std::string path, std::string callback, int headers)
{
    ClientData cd;
    try {
        cd = clientsTable[id];
    } catch (std::exception e) {
        return 1;
    }

    http_request request(methods::GET);
	for (auto h : cd.headers) {
		request.headers().add(
			utility::conversions::to_string_t(h.first),
			utility::conversions::to_string_t(h.second));
	}
    for (auto h : headersTable[headers]) {
        request.headers().add(
            utility::conversions::to_string_t(h.first),
            utility::conversions::to_string_t(h.second));
    }
    request.set_request_uri(utility::conversions::to_string_t(path));

    cd.client->request(request).then([=](http_response response) {
        taskStackLock.lock();
        taskStack.push([&]() {
            CallbackTask t;
            t.id = requestCounter;
            t.callback = callback;
            t.type = E_TASK_TYPE::string;
            t.status = response.status_code();
            t.string = response.extract_utf8string().get();
            return t;
        }());
        taskStackLock.unlock();
        return;
    });

    return requestCounter++;
}

int Impl::RestfulPostData(int id, std::string endpoint, std::string callback, char* data, int headers)
{
    return requestCounter++;
}

int Impl::RestfulGetJSON(int id, std::string endpoint, std::string callback, int headers)
{
    return requestCounter++;
}

int Impl::RestfulPostJSON(int id, std::string endpoint, std::string callback, web::json::object json, int headers)
{
    return requestCounter++;
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
    if (taskStackLock.try_lock()) {
        CallbackTask cbt;
        while (!taskStack.empty()) {
            cbt = taskStack.top();
            tasks.push_back(cbt);
            taskStack.pop();
        }
        taskStackLock.unlock();
    }

    return tasks;
}
