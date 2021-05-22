#pragma once

#include "stdafx.h"
#include "lua_ptr.h"
#include <tlhelp32.h>

#define MODULE_T MEMREADER_METATABLE(module)

typedef struct
{
	HMODULE handle;
	CHAR name[MAX_MODULE_NAME32 + 1];
	CHAR path[MAX_PATH];
	DWORD size;
} module_t;

void init_module(module_t *module, MODULEENTRY32 *me32);
module_t *push_module(lua_State *L);

int register_module(lua_State *L);