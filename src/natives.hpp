/*
# natives.hpp

Contains all the `PAWN_NATIVE_DECL` for native function declarations.
*/

#ifndef RESTFUL_NATIVES_H
#define RESTFUL_NATIVES_H

#include <string>

#include <amx/amx2.h>

#include "common.hpp"
#include "plugin-natives\NativeFunc.hpp" // must be included last

PAWN_NATIVE_DECL(restful, Function, bool())

#endif
