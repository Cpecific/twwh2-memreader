#include "stdafx.h"
#include "lua_ptr.h"
#include "utils.h"
#include "module.h"

#include <psapi.h>

void init_module(module_t *module, MODULEENTRY32 *me32)
{
	module->handle = me32->hModule;
	memcpy_s(module->name, sizeof(module->name), me32->szModule, sizeof(me32->szModule));
	memcpy_s(module->path, sizeof(module->path), me32->szExePath, sizeof(me32->szExePath));
	module->size = me32->modBaseSize;
}

module_t *push_module(lua_State *L)
{
	module_t *mod = (module_t *)lua_newuserdata(L, sizeof(module_t));
	luaL_getmetatable(L, MODULE_T);
	lua_setmetatable(L, -2);
	return mod;
}

const luaL_Reg module_meta[] = {
	{NULL, NULL}};
const luaL_Reg module_methods[] = {
	{NULL, NULL}};
udata_field_info module_getters[] = {
	{"name", udata_field_get_string, offsetof(module_t, name)},
	{"path", udata_field_get_string, offsetof(module_t, path)},
	{"base", udata_field_get_memaddress, offsetof(module_t, handle)},
	{"size", udata_field_get_int, offsetof(module_t, size)},
	{NULL, NULL}};
udata_field_info module_setters[] = {
	{NULL, NULL}};

int register_module(lua_State *L)
{
	UDATA_REGISTER_TYPE_WITH_FIELDS(module, MODULE_T);
	return 0;
}