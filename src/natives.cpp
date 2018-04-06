/*
# natives.cpp

This source file contains the bridge between natives and implementations. I
prefer to keep the actual implementation separate. The implementation contains
no instances of `cell` or `AMX*` and is purely C++ and external library code.
The code here acts as the translation between AMX data types and native types.
*/

#include "natives.hpp"

PAWN_NATIVE_DEFN(restful, Function, bool())
{
    logprintf("Function called");
    std::string s;
    return 0;
}
