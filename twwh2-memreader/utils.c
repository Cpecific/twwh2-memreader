#include "stdafx.h"
#include "utils.h"
#include "memreader.h"

int push_error(lua_State *L, const char *msg)
{
	lua_pushnil(L);
	lua_pushstring(L, msg);
	return 2;
}

int push_last_error(lua_State *L, const char *msg)
{
	return luaL_error(L, msg);
	// return push_error(L, "failed to read memory");
	// static char err[256];
	// int strLen = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
	// 							NULL,
	// 							GetLastError(),
	// 							0x0009, // GetSystemDefaultLangID(), // Default language
	// 							err,
	// 							256, NULL);
	// if (strlen >= 2)
	// 	err[strLen - 2] = 0; // strip the \r\n
	// else
	// 	err[0] = 0;
	// return push_error(L, err);
	// return luaL_error(L, err);
}

const char *get_lua_string(lua_State *L, int index)
{
	const char *s;
	lua_getglobal(L, "tostring");
	lua_pushvalue(L, index);
	lua_call(L, 1, 1);
	s = lua_tostring(L, -1);
	lua_pop(L, 1);
	return s;
}

// Userdata Fields

int udata_field_get_int(lua_State *L, void *v)
{
	lua_pushinteger(L, *(int *)v);
	return 1;
}
int udata_field_set_int(lua_State *L, void *v)
{
	*(int *)v = (int)luaL_checkinteger(L, 3);
	return 0;
}

int udata_field_get_string(lua_State *L, void *v)
{
	lua_pushstring(L, (const char *)v);
	return 1;
}
int udata_field_set_string(lua_State *L, void *v)
{
	v = (void *)luaL_checkstring(L, 3);
	return 0;
}
int udata_field_get_memaddress(lua_State *L, void *v)
{
	lmr_push_userdata(L, LUA_TPOINTER)->p = *(LPVOID *)v;
	return 1;
}

void register_udata_fields(lua_State *L, udata_field_reg l)
{
	for (; l->name; l++)
	{
		lua_pushstring(L, l->name);
		lua_pushlightuserdata(L, (void *)l);
		lua_settable(L, -3);
	}
}

int udata_field_proxy(lua_State *L)
{
	/* for get: stack has userdata, index, lightuserdata */
	/* for set: stack has userdata, index, value, lightuserdata */
	udata_field_info *m = (udata_field_info *)lua_touserdata(L, -1); /* member info */
	lua_pop(L, 1);													 /* drop lightuserdata */
	luaL_checktype(L, 1, LUA_TUSERDATA);
	return m->func(L, (void *)((char *)lua_touserdata(L, 1) + m->offset));
}

int udata_field_index_handler(lua_State *L)
{
	/* stack has userdata, index */
	lua_pushvalue(L, 2);				/* dup index */
	lua_rawget(L, lua_upvalueindex(1)); /* lookup member by name */
	if (!lua_islightuserdata(L, -1))
	{
		lua_pop(L, 1);						  /* drop value */
		lua_pushvalue(L, 2);				  /* dup index */
		lua_gettable(L, lua_upvalueindex(2)); /* else try methods */
		return 1;							  // return either the method or nil
	}
	return udata_field_proxy(L); /* call get function */
}

int udata_field_newindex_handler(lua_State *L)
{
	/* stack has userdata, index, value */
	lua_pushvalue(L, 2);				/* dup index */
	lua_rawget(L, lua_upvalueindex(1)); /* lookup member by name */
	if (!lua_islightuserdata(L, -1))	/* invalid member */
	{
		luaL_error(L, "attempt to set field '%s' of %s", lua_tostring(L, 2), get_lua_string(L, 1));
	}
	return udata_field_proxy(L); /* call set function */
}