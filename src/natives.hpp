/*
# natives.hpp

Contains all the `PAWN_NATIVE_DECL` for native function declarations.
*/

#ifndef RESTFUL_NATIVES_H
#define RESTFUL_NATIVES_H

#include <string>

#include <amx/amx2.h>

#include "plugin-natives\NativeFunc.hpp"

#include "common.hpp"

PAWN_NATIVE_DECL(restful, Function, bool())

#endif
