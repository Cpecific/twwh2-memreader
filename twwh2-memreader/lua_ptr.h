#pragma once

#define lua_call lua_call__unref
#define lua_createtable lua_createtable__unref
#define lua_getfield lua_getfield__unref
#define lua_gettable lua_gettable__unref
#define lua_gettop lua_gettop__unref
#define lua_newuserdata lua_newuserdata__unref
#define lua_pushboolean lua_pushboolean__unref
#define lua_pushcclosure lua_pushcclosure__unref
#define lua_pushfstring lua_pushfstring__unref
#define lua_pushinteger lua_pushinteger__unref
#define lua_pushlstring lua_pushlstring__unref
#define lua_pushlightuserdata lua_pushlightuserdata__unref
#define lua_pushnil lua_pushnil__unref
#define lua_pushnumber lua_pushnumber__unref
#define lua_pushstring lua_pushstring__unref
#define lua_pushvalue lua_pushvalue__unref
#define lua_rawget lua_rawget__unref
#define lua_rawset lua_rawset__unref
#define lua_replace lua_replace__unref
#define lua_setfield lua_setfield__unref
#define lua_setmetatable lua_setmetatable__unref
#define lua_settable lua_settable__unref
#define lua_settop lua_settop__unref
#define lua_toboolean lua_toboolean__unref
#define lua_tointeger lua_tointeger__unref
#define lua_tolstring lua_tolstring__unref
#define lua_tonumber lua_tonumber__unref
#define lua_topointer lua_topointer__unref
#define lua_touserdata lua_touserdata__unref
#define lua_type lua_type__unref
#define lua_typename lua_typename__unref

#define luaL_callmeta luaL_callmeta__unref
#define luaL_checkany luaL_checkany__unref
#define luaL_checkinteger luaL_checkinteger__unref
#define luaL_checklstring luaL_checklstring__unref
#define luaL_checktype luaL_checktype__unref
#define luaL_checkudata luaL_checkudata__unref
#define luaL_error luaL_error__unref
#define luaL_newmetatable luaL_newmetatable__unref
#define luaL_register luaL_register__unref

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lobject.h"
#include "lstate.h"

// #if LUA_VERSION_NUM < 502
#ifndef luaL_newlib

// #define luaL_newlib(L, l) (lua_newtable(L), luaL_register(L, NULL, l))
#define luaL_newlibtable(L, l) lua_createtable(L, 0, sizeof(l) / sizeof((l)[0]) - 1)
#define luaL_newlib(L, l) (luaL_newlibtable(L, l), luaL_setfuncs(L, l, 0))

#endif
#define luaL_setfuncs(L, l, n) (assert(n == 0), luaL_register(L, NULL, l))
// api_check(L, L->top < L->ci->top);
#define api_incr_top(L) L->top++;
#define get_udata(o) ((lmr_Value *)(&(o)->value.gc->u + 1))
// #endif

#define LUA_TPOINTER 0
#define LUA_TUINT8 1
#define LUA_TINT8 2
#define LUA_TUINT16 3
#define LUA_TINT16 4
#define LUA_TUINT32 5
#define LUA_TINT32 6

typedef struct
{
	BYTE tt;
	union
	{
		// char *p;
		INT_PTR p;
		UINT8 uint8;
		INT8 int8;
		UINT16 uint16;
		INT16 int16;
		UINT32 uint32;
		INT32 int32;
	};
} lmr_Value;

#undef lua_call
#undef lua_createtable
#undef lua_getfield
#undef lua_gettable
#undef lua_gettop
#undef lua_newuserdata
#undef lua_pushboolean
#undef lua_pushcclosure
#undef lua_pushfstring
#undef lua_pushinteger
#undef lua_pushlightuserdata
#undef lua_pushlstring
#undef lua_pushnil
#undef lua_pushnumber
#undef lua_pushstring
#undef lua_pushvalue
#undef lua_rawget
#undef lua_rawset
#undef lua_replace
#undef lua_setfield
#undef lua_setmetatable
#undef lua_settable
#undef lua_settop
#undef lua_toboolean
#undef lua_tointeger
#undef lua_tolstring
#undef lua_tonumber
#undef lua_topointer
#undef lua_touserdata
#undef lua_type
#undef lua_typename

#undef luaL_callmeta
#undef luaL_checkany
#undef luaL_checkinteger
#undef luaL_checklstring
#undef luaL_checktype
#undef luaL_checkudata
#undef luaL_error
#undef luaL_newmetatable
#undef luaL_register

extern void (*lua_call)(lua_State *L, int nargs, int nresults);
extern void (*lua_createtable)(lua_State *L, int narr, int nrec);
extern void (*lua_getfield)(lua_State *L, int idx, const char *k);
extern void (*lua_gettable)(lua_State *L, int idx);
extern int (*lua_gettop)(lua_State *L);
extern void *(*lua_newuserdata)(lua_State *L, size_t sz);
extern void (*lua_pushboolean)(lua_State *L, int b);
extern void (*lua_pushcclosure)(lua_State *L, lua_CFunction fn, int n);
extern const char *(*lua_pushfstring)(lua_State *L, const char *fmt, ...);
extern void (*lua_pushinteger)(lua_State *L, lua_Integer n);
extern void (*lua_pushlightuserdata)(lua_State *L, void *p);
extern void (*lua_pushlstring)(lua_State *L, const char *s, size_t l);
extern void (*lua_pushnil)(lua_State *L);
extern void (*lua_pushnumber)(lua_State *L, lua_Number n);
extern void (*lua_pushstring)(lua_State *L, const char *s);
extern void (*lua_pushvalue)(lua_State *L, int idx);
extern void (*lua_rawget)(lua_State *L, int idx);
extern void (*lua_rawset)(lua_State *L, int idx);
extern void (*lua_replace)(lua_State *L, int idx);
extern void (*lua_setfield)(lua_State *L, int idx, const char *k);
extern int (*lua_setmetatable)(lua_State *L, int objindex);
extern void (*lua_settable)(lua_State *L, int idx);
extern void (*lua_settop)(lua_State *L, int idx);
extern int (*lua_toboolean)(lua_State *L, int idx);
extern lua_Integer (*lua_tointeger)(lua_State *L, int idx);
extern const char *(*lua_tolstring)(lua_State *L, int idx, size_t *len);
extern lua_Number (*lua_tonumber)(lua_State *L, int idx);
extern const void *(*lua_topointer)(lua_State *L, int idx);
extern void *(*lua_touserdata)(lua_State *L, int idx);
extern int (*lua_type)(lua_State *L, int idx);
extern const char *(*lua_typename)(lua_State *L, int tp);

extern int (*luaL_callmeta)(lua_State *L, int obj, const char *e);
extern void (*luaL_checkany)(lua_State *L, int narg);
extern lua_Integer (*luaL_checkinteger)(lua_State *L, int numArg);
extern const char *(*luaL_checklstring)(lua_State *L, int numArg, size_t *l);
extern void (*luaL_checktype)(lua_State *L, int narg, int t);
extern void *(*luaL_checkudata)(lua_State *L, int ud, const char *tname);
extern int (*luaL_error)(lua_State *L, const char *fmt, ...);
extern int (*luaL_newmetatable)(lua_State *L, const char *tname);
extern void (*luaL_register)(lua_State *L, const char *libname, const luaL_Reg *l);

extern void init_lua_ptr(lua_State *L);