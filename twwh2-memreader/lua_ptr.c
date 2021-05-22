#include "stdafx.h"
#include "lua_ptr.h"

#ifdef _WIN32
#include <stdio.h>
#include <stdlib.h>
#else
#include <dlfcn.h>
#define GetProcAddress dlsym
#endif

void (*lua_call)(lua_State *L, int nargs, int nresults);
void (*lua_createtable)(lua_State *L, int narr, int nrec);
void (*lua_getfield)(lua_State *L, int idx, const char *k);
void (*lua_gettable)(lua_State *L, int idx);
int (*lua_gettop)(lua_State *L);
void *(*lua_newuserdata)(lua_State *L, size_t sz);
void (*lua_pushboolean)(lua_State *L, int b);
void (*lua_pushcclosure)(lua_State *L, lua_CFunction fn, int n);
const char *(*lua_pushfstring)(lua_State *L, const char *fmt, ...);
void (*lua_pushinteger)(lua_State *L, lua_Integer n);
void (*lua_pushlightuserdata)(lua_State *L, void *p);
void (*lua_pushlstring)(lua_State *L, const char *s, size_t l);
void (*lua_pushnil)(lua_State *L);
void (*lua_pushnumber)(lua_State *L, lua_Number n);
void (*lua_pushstring)(lua_State *L, const char *s);
void (*lua_pushvalue)(lua_State *L, int idx);
void (*lua_rawget)(lua_State *L, int idx);
void (*lua_rawset)(lua_State *L, int idx);
void (*lua_replace)(lua_State *L, int idx);
void (*lua_setfield)(lua_State *L, int idx, const char *k);
int (*lua_setmetatable)(lua_State *L, int objindex);
void (*lua_settable)(lua_State *L, int idx);
void (*lua_settop)(lua_State *L, int idx);
int (*lua_toboolean)(lua_State *L, int idx);
lua_Integer (*lua_tointeger)(lua_State *L, int idx);
const char *(*lua_tolstring)(lua_State *L, int idx, size_t *len);
lua_Number (*lua_tonumber)(lua_State *L, int idx);
const void *(*lua_topointer)(lua_State *L, int idx);
void *(*lua_touserdata)(lua_State *L, int idx);
int (*lua_type)(lua_State *L, int idx);
const char *(*lua_typename)(lua_State *L, int tp);

int (*luaL_callmeta)(lua_State *L, int obj, const char *e);
void (*luaL_checkany)(lua_State *L, int narg);
lua_Integer (*luaL_checkinteger)(lua_State *L, int numArg);
const char *(*luaL_checklstring)(lua_State *L, int numArg, size_t *l);
void (*luaL_checktype)(lua_State *L, int narg, int t);
void *(*luaL_checkudata)(lua_State *L, int ud, const char *tname);
int (*luaL_error)(lua_State *L, const char *fmt, ...);
int (*luaL_newmetatable)(lua_State *L, const char *tname);
void (*luaL_register)(lua_State *L, const char *libname, const luaL_Reg *l);

void init_lua_ptr(lua_State *L)
{
#ifdef _WIN32
	HMODULE module;
	module = GetModuleHandleA(NULL);
#else
	void *module;
	module = dlopen(0, RTLD_LAZY); // NULL
#endif

	lua_call = GetProcAddress(module, "lua_call");
	lua_createtable = GetProcAddress(module, "lua_createtable");
	lua_getfield = GetProcAddress(module, "lua_getfield");
	lua_gettable = GetProcAddress(module, "lua_gettable");
	lua_gettop = GetProcAddress(module, "lua_gettop");
	lua_newuserdata = GetProcAddress(module, "lua_newuserdata");
	lua_pushboolean = GetProcAddress(module, "lua_pushboolean");
	lua_pushcclosure = GetProcAddress(module, "lua_pushcclosure");
	lua_pushfstring = GetProcAddress(module, "lua_pushfstring");
	lua_pushinteger = GetProcAddress(module, "lua_pushinteger");
	lua_pushlightuserdata = GetProcAddress(module, "lua_pushlightuserdata");
	lua_pushlstring = GetProcAddress(module, "lua_pushlstring");
	lua_pushnil = GetProcAddress(module, "lua_pushnil");
	lua_pushnumber = GetProcAddress(module, "lua_pushnumber");
	lua_pushstring = GetProcAddress(module, "lua_pushstring");
	lua_pushvalue = GetProcAddress(module, "lua_pushvalue");
	lua_rawget = GetProcAddress(module, "lua_rawget");
	lua_rawset = GetProcAddress(module, "lua_rawset");
	lua_replace = GetProcAddress(module, "lua_replace");
	lua_setfield = GetProcAddress(module, "lua_setfield");
	lua_setmetatable = GetProcAddress(module, "lua_setmetatable");
	lua_settable = GetProcAddress(module, "lua_settable");
	lua_settop = GetProcAddress(module, "lua_settop");
	lua_toboolean = GetProcAddress(module, "lua_toboolean");
	lua_tointeger = GetProcAddress(module, "lua_tointeger");
	lua_tolstring = GetProcAddress(module, "lua_tolstring");
	lua_tonumber = GetProcAddress(module, "lua_tonumber");
	lua_topointer = GetProcAddress(module, "lua_topointer");
	lua_touserdata = GetProcAddress(module, "lua_touserdata");
	lua_type = GetProcAddress(module, "lua_type");
	lua_typename = GetProcAddress(module, "lua_typename");

	luaL_callmeta = GetProcAddress(module, "luaL_callmeta");
	luaL_checkany = GetProcAddress(module, "luaL_checkany");
	luaL_checkinteger = GetProcAddress(module, "luaL_checkinteger");
	luaL_checklstring = GetProcAddress(module, "luaL_checklstring");
	luaL_checktype = GetProcAddress(module, "luaL_checktype");
	luaL_checkudata = GetProcAddress(module, "luaL_checkudata");
	luaL_error = GetProcAddress(module, "luaL_error");
	luaL_newmetatable = GetProcAddress(module, "luaL_newmetatable");
	luaL_register = GetProcAddress(module, "luaL_register");
}