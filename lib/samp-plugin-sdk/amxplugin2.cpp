//----------------------------------------------------------
//
//   SA-MP Multiplayer Modification For GTA:SA
//   Copyright 2014 SA-MP Team, Dan, maddinat0r
//
//----------------------------------------------------------

#include <cstring>
#include <cstdlib>

//----------------------------------------------------------

#include <assert.h>

//----------------------------------------------------------

#include "amx/amx2.h"

//----------------------------------------------------------

int AMXAPI amx_PushAddress(AMX *amx, cell *address) 
{
	AMX_HEADER *hdr;
	unsigned char *data;
	cell xaddr;
	/* reverse relocate the address */
	assert(amx != NULL);
	hdr = (AMX_HEADER *) amx->base;
	assert(hdr != NULL);
	assert(hdr->magic == AMX_MAGIC);
	data = (amx->data != NULL) ? amx->data : amx->base + (int) hdr->dat;
	xaddr = (cell) ((unsigned char*) address-data);
	if ((ucell) xaddr >= (ucell) amx->stp) 
	{
		return AMX_ERR_MEMACCESS;
	}
	return amx_Push(amx,xaddr);
}

void AMXAPI amx_Redirect(AMX *amx, char *from, ucell to, AMX_NATIVE *store) 
{
	AMX_HEADER *hdr = (AMX_HEADER*) amx->base;
	AMX_FUNCSTUB *func;
	for (int idx = 0, num = NUMENTRIES(hdr, natives, libraries); idx != num; ++idx) 
	{
		func = GETENTRY(hdr, natives, idx);
		if (!strcmp(from, GETENTRYNAME(hdr, func))) 
		{
			if (store) 
			{
				*store = (AMX_NATIVE) func->address;
			}
			func->address = to;
			return;
		}
	}
}

int AMXAPI amx_GetCString(AMX *amx, cell param, char *&dest) 
{
	cell *ptr;
	amx_GetAddr(amx, param, &ptr);
	int len;
	amx_StrLen(ptr, &len);
	dest = (char*) malloc((len + 1) * sizeof(char));
	if (dest != NULL) 
	{
		amx_GetString(dest, ptr, 0, UNLIMITED);
		dest[len] = 0;
		return len;
	}
	return 0;
}

int AMXAPI amx_SetCString(AMX *amx, cell param, const char *str, int len) 
{
	cell *dest;
	int error;
	if ((error = amx_GetAddr(amx, param, &dest)) != AMX_ERR_NONE)
		return error;

	return amx_SetString(dest, str, 0, 0, len);
}

#if defined __cplusplus

std::string AMXAPI amx_GetCppString(AMX *amx, cell param) 
{
	cell *addr = nullptr;
	amx_GetAddr(amx, param, &addr);

	int len = 0;
	amx_StrLen(addr, &len);

	std::string string(len, ' ');
	amx_GetString(&string[0], addr, 0, len + 1);

	return string;
}

int AMXAPI amx_SetCppString(AMX *amx, cell param, const std::string &str, size_t maxlen)
{
	cell *dest = nullptr;
	int error;
	if ((error = amx_GetAddr(amx, param, &dest)) != AMX_ERR_NONE)
		return error;

	return amx_SetString(dest, str.c_str(), 0, 0, maxlen);
}

#endif // __cplusplus

//----------------------------------------------------------
// EOF
