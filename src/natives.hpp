/*
# natives.hpp

Contains all the `PAWN_NATIVE_DECL` for native function declarations.
*/

#ifndef RESTFUL_NATIVES_H
#define RESTFUL_NATIVES_H

#include <set>
#include <string>
#include <utility>

#include <amx/amx2.h>
#include <cpprest/json.h>

#include "common.hpp"
// #include "plugin-natives\NativeFunc.hpp" // must be included last

namespace Natives {
int RestfulGetData(AMX* amx, cell* params);
int RestfulPostData(AMX* amx, cell* params);
int RestfulGetJSON(AMX* amx, cell* params);
int RestfulPostJSON(AMX* amx, cell* params);
int RestfulHeaders(AMX* amx, cell* params);

namespace JSON {
    int Object(AMX* amx, cell* params);
    int String(AMX* amx, cell* params);
    int Int(AMX* amx, cell* params);
    int Float(AMX* amx, cell* params);
    int Array(AMX* amx, cell* params);
    int Stringify(AMX* amx, cell* params);
    int Cleanup(AMX* amx, cell* params);

    // this is now a pointer to json value, upon cleanup, delete the value
    // no need to invalidate the pool ID as they are ephemeral across function calls.
    extern std::unordered_map<int, web::json::value*> jsonPool;
    extern int jsonPoolCounter;
    int Alloc(web::json::value item);
    web::json::value Get(int id);
}
}

#endif
