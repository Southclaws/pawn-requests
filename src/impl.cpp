/*
# impl.cpp

As with the header, this is the actual implementation of the plugin's
functionality with no AMX specific code or includes.

Including `common.hpp` for access to `logprintf` is useful for debugging but for
production debug logging, it's best to use a dedicated logging library such as
log-core by maddinat0r.
*/

#include "impl.hpp"

int Impl::RestfulGetData(std::string endpoint, std::string callback, std::vector<std::string> headers)
{
    return 0;
}

int Impl::RestfulPostData(std::string endpoint, std::string callback, char* data, std::vector<std::string> headers)
{
    return 0;
}

int Impl::RestfulGetJSON(std::string endpoint, std::string callback, std::vector<std::string> headers)
{
    return 0;
}

int Impl::RestfulPostJSON(std::string endpoint, std::string callback, web::json::object json, std::vector<std::string> headers)
{
    return 0;
}

int Impl::RestfulHeaders(std::string...)
{
    return 0;
}
