/*
# impl.hpp

From here on, it's just regular C++ code, no AMX/Pawn/SA:MP stuff. This header
is for declaring implementation functions for the plugin's core functionality.
*/

#include <curl.h>

#ifndef RESTFUL_IMPL_H
#define RESTFUL_IMPL_H

namespace Impl {
	int HttpGet();
};

#endif
