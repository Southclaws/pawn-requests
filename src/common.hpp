/*
# common.hpp

This simply contains some AMX pointer definitions and the logprintf typedef.
*/

extern void** ppPluginData;
extern void* pAMXFunctions;
typedef void (*logprintf_t)(const char* szFormat, ...);
extern logprintf_t logprintf;
