/*
# impl.cpp

As with the header, this is the actual implementation of the plugin's
functionality with no AMX specific code or includes.

Including `common.hpp` for access to `logprintf` is useful for debugging but for
production debug logging, it's best to use a dedicated logging library such as
log-core by maddinat0r.
*/

#include "impl.hpp"

int Impl::HttpGet() {
	web::http::client::http_client_config client_config;
	http_client client(U("http://api.samp.southcla.ws/v2/servers/"), client_config);
	client.request(methods::GET, "", "").wait();

	return 0;
}
