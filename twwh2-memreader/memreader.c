#include "memreader.h"
#include "module.h"
#include "utils.h"

#include <psapi.h>
#include <tlhelp32.h>

#include <assert.h>

// TODO will lua be fine with negative tt? because it does go wanky with gc, when tt >= LUA_TSTRING
// TODO clone function? nah, you can just add(value, 0)

// default x64 struct member alignment
static_assert(offsetof(lmr_Value, p) == 8, "");
static_assert(offsetof(lmr_Value, uint8) == 8, "");
static_assert(offsetof(lmr_Value, int8) == 8, "");
static_assert(offsetof(lmr_Value, uint16) == 8, "");
static_assert(offsetof(lmr_Value, int16) == 8, "");
static_assert(offsetof(lmr_Value, uint32) == 8, "");
static_assert(offsetof(lmr_Value, int32) == 8, "");

lmr_Value *lmr_push_userdata(lua_State *L, BYTE tt)
{
	lmr_Value *p_udata = (lmr_Value *)lua_newuserdata(L, sizeof(lmr_Value));
	p_udata->tt = tt;
	return p_udata;
}
LPVOID lmr_string_to_pointer(const TValue *o)
{
	const char *p = svalue(o);
	// if (sizeof(LPVOID) == 8)
	return *(UINT64 *)p;
	// else
	// 	return *(UINT32 *)p;
}
UINT32 lmr_string_to_uint32(const TValue *o)
{
	const char *p = svalue(o);
	return *(UINT32 *)p;
}
LPVOID lmr_to_pointer(lua_State *L, const TValue *o)
{
	lmr_Value *u;
	switch (o->tt)
	{
	case LUA_TSTRING:
		return lmr_string_to_pointer(o);
	case LUA_TUSERDATA:
		u = get_udata(o);
		switch (u->tt)
		{
		case LUA_TPOINTER:
			return u->p;
		}
	}
	return luaL_error(L, "expected pointer argument in the form of raw string or pointer type");
}
size_t lmr_to_size_t(lua_State *L, const TValue *o)
{
	lmr_Value *u;
	if (o >= L->top)
		return 0;
	switch (o->tt)
	{
	case LUA_TNUMBER:
		return o->value.n;
	case LUA_TSTRING:
		return lmr_string_to_uint32(o);
	case LUA_TUSERDATA:
		u = get_udata(o);
		switch (u->tt)
		{
		// case LUA_TPOINTER:
		// 	return u->p;
		// case LUA_TUINT8:
		// 	return u->uint8;
		// case LUA_TINT8:
		// 	return u->int8;
		// case LUA_TUINT16:
		// 	return u->uint16;
		// case LUA_TINT16:
		// 	return u->int16;
		case LUA_TUINT32:
			return u->uint32;
		case LUA_TINT32:
			return u->int32;
		}
	}
	return 0;
}
BOOL lmr_to_boolean(lua_State *L, const TValue *o)
{
	if (o >= L->top)
		return FALSE;
	if (o->tt == LUA_TBOOLEAN)
		return o->value.b;
	return FALSE;
}

int memreader_snapshot_gc(lua_State *L)
{
	HANDLE handle = *(HANDLE *)lua_touserdata(L, 1);
	if (handle)
		CloseHandle(handle);
	return 0;
}
int register_snapshot(lua_State *L)
{
	// luaL_newmetatable(L, SNAPSHOT_T); // registry[SNAPSHOT_T] = snapshot_metatable
	luaL_setmetatable(L, SNAPSHOT_T); // registry[SNAPSHOT_T] = snapshot_metatable
	lua_pushstring(L, "__gc");
	lua_pushcfunction(L, memreader_snapshot_gc);
	lua_settable(L, -3); // snapshot_metatable[__gc] = memreader_snapshot_gc
	lua_pop(L, 1);
	return 0;
}

static const char *p_typeError = "passed invalid argument type";
static const char *p_readError = "failed to read memory";
static const char *p_writeError = "failed to write memory";
static char read_buffer[1024];
int l_read_float(lua_State *L)
{
	char *address = lmr_to_pointer(L, L->base);
	SIZE_T offset = lmr_to_size_t(L, L->base + 1);

	HANDLE handle = GetCurrentProcess();
	lua_Number tmp;
	BOOL success = ReadProcessMemory(handle, address + offset, &tmp, sizeof(lua_Number), NULL);
	if (!success)
		return push_last_error(L, p_readError);
	lua_pushnumber(L, tmp);
	return 1;
}
int l_read_pointer(lua_State *L)
{
	char *address = lmr_to_pointer(L, L->base);
	SIZE_T offset = lmr_to_size_t(L, L->base + 1);
	address += offset;

	HANDLE handle = GetCurrentProcess(); // no need to close
	lmr_Value *u = lmr_push_userdata(L, LUA_TPOINTER);
	BOOL success = ReadProcessMemory(handle, address, &u->p, sizeof(LPVOID), NULL);
	if (!success)
		return push_last_error(L, p_readError);
	return 1;
}
BOOL read_some_number(lua_State *L, SIZE_T size)
{
	char *address = lmr_to_pointer(L, L->base);
	SIZE_T offset = lmr_to_size_t(L, L->base + 1);

	HANDLE handle = GetCurrentProcess();
	return ReadProcessMemory(handle, address + offset, read_buffer, size, NULL);
}
int l_read_uint8(lua_State *L)
{
	if (!read_some_number(L, sizeof(UINT8)))
		return push_last_error(L, p_readError);

	BOOL toUserdata = lmr_to_boolean(L, L->base + 2);
	UINT8 value = *((UINT8 *)read_buffer);
	if (toUserdata)
		lmr_push_userdata(L, LUA_TUINT8)->uint8 = value;
	else
		lua_pushnumber(L, value);
	return 1;
}
int l_read_int8(lua_State *L)
{
	if (!read_some_number(L, sizeof(INT8)))
		return push_last_error(L, p_readError);

	BOOL toUserdata = lmr_to_boolean(L, L->base + 2);
	INT8 value = *((INT8 *)read_buffer);
	if (toUserdata)
		lmr_push_userdata(L, LUA_TINT8)->int8 = value;
	else
		lua_pushnumber(L, value);
	return 1;
}
int l_read_uint16(lua_State *L)
{
	if (!read_some_number(L, sizeof(UINT16)))
		return push_last_error(L, p_readError);

	BOOL toUserdata = lmr_to_boolean(L, L->base + 2);
	UINT16 value = *((UINT16 *)read_buffer);
	if (toUserdata)
		lmr_push_userdata(L, LUA_TUINT16)->uint16 = value;
	else
		lua_pushnumber(L, value);
	return 1;
}
int l_read_int16(lua_State *L)
{
	if (!read_some_number(L, sizeof(INT16)))
		return push_last_error(L, p_readError);

	BOOL toUserdata = lmr_to_boolean(L, L->base + 2);
	INT16 value = *((INT16 *)read_buffer);
	if (toUserdata)
		lmr_push_userdata(L, LUA_TINT16)->int16 = value;
	else
		lua_pushnumber(L, value);
	return 1;
}
int l_read_uint32(lua_State *L)
{
	if (!read_some_number(L, sizeof(UINT32)))
		return push_last_error(L, p_readError);

	BOOL toUserdata = lmr_to_boolean(L, L->base + 2);
	UINT32 value = *((UINT32 *)read_buffer);
	if (toUserdata)
		lmr_push_userdata(L, LUA_TUINT32)->uint32 = value;
	else
		lua_pushnumber(L, value);
	return 1;
}
int l_read_int32(lua_State *L)
{
	if (!read_some_number(L, sizeof(INT32)))
		return push_last_error(L, p_readError);

	BOOL toUserdata = lmr_to_boolean(L, L->base + 2);
	INT32 value = *((INT32 *)read_buffer);
	if (toUserdata)
		lmr_push_userdata(L, LUA_TINT32)->int32 = value;
	else
		lua_pushnumber(L, value);
	return 1;
}

int l_read_boolean(lua_State *L)
{
	char *address = lmr_to_pointer(L, L->base);
	SIZE_T offset = lmr_to_size_t(L, L->base + 1);

	HANDLE handle = GetCurrentProcess();
	BYTE b;
	BOOL success = ReadProcessMemory(handle, address + offset, &b, sizeof(BYTE), NULL);
	if (!success)
		return push_last_error(L, p_readError);

	lua_pushboolean(L, b == 0 ? 0 : 1);
	return 1;
}
int l_read_string(lua_State *L)
{
	char *address = lmr_to_pointer(L, L->base);
	SIZE_T offset = lmr_to_size_t(L, L->base + 1);
	address += offset;
	BOOL isPtr = lmr_to_boolean(L, L->base + 2);
	BOOL isWide = lmr_to_boolean(L, L->base + 3);

	BOOL success;
	HANDLE handle = GetCurrentProcess();
	if (isPtr)
	{
		success = ReadProcessMemory(handle, address, &address, sizeof(LPVOID), NULL);
		if (!success)
			return push_last_error(L, p_readError);
	}

	INT32 length;
	success = ReadProcessMemory(handle, address, &length, sizeof(INT32), NULL);
	if (!success)
		return push_last_error(L, p_readError);

	if (length <= 0)
	{
		lua_pushliteral(L, "");
		return 1;
	}
	success = ReadProcessMemory(handle, address + 0x08, &address, sizeof(LPVOID), NULL);
	if (!success)
		return push_last_error(L, p_readError);

	if (isWide)
		length *= 2;

	char *p;
	if (length < sizeof(read_buffer))
		p = read_buffer;
	else
		p = malloc(length);

	success = ReadProcessMemory(handle, address, p, length, NULL);
	if (!success)
	{
		if (p != read_buffer)
			free(p);
		return push_last_error(L, p_readError);
	}

	lua_pushlstring(L, p, length);
	if (p != read_buffer)
		free(p);
	return 1;
}
int l_read_array(lua_State *L)
{
	char *address = lmr_to_pointer(L, L->base);
	SIZE_T offset = lmr_to_size_t(L, L->base + 1);
	address += offset;
	BOOL toUserdata = lmr_to_boolean(L, L->base + 2);

	BOOL success;
	HANDLE handle = GetCurrentProcess();

	INT32 size;
	success = ReadProcessMemory(handle, address + 0x04, &size, sizeof(INT32), NULL);
	if (!success)
		return push_last_error(L, p_readError);

	if (toUserdata)
		lmr_push_userdata(L, LUA_TINT32)->int32 = size;
	else
		lua_pushnumber(L, size);
	lmr_Value *uptr = lmr_push_userdata(L, LUA_TPOINTER);
	if (size <= 0)
	{
		uptr->p = NULL;
		return 2;
	}
	success = ReadProcessMemory(handle, address + 0x08, &uptr->p, sizeof(LPVOID), NULL);
	if (!success)
		return push_last_error(L, p_readError);

	return 2;
}
int l_read_rowidx(lua_State *L)
{
	char *address = lmr_to_pointer(L, L->base);
	SIZE_T offset = lmr_to_size_t(L, L->base + 1);
	address += offset;
	char *base = lmr_to_pointer(L, L->base + 2);
	lua_Number row_size = lua_tonumber(L, 4);

	HANDLE handle = GetCurrentProcess();
	BOOL success = ReadProcessMemory(handle, address, &address, sizeof(LPVOID), NULL);
	if (!success)
		return push_last_error(L, p_readError);

	ptrdiff_t diff = address - base;
	diff /= row_size;
	lua_pushnumber(L, diff + 1);
	return 1;
}
int l_read(lua_State *L)
{
	char *address = lmr_to_pointer(L, L->base);
	SIZE_T offset = lmr_to_size_t(L, L->base + 1);
	// SIZE_T bytes = lmr_to_size_t(L, L->base + 2);
	SIZE_T bytes = lua_tonumber(L, 3);

	if (bytes == 0)
	{
		lua_pushliteral(L, "");
		return 1;
	}

	SIZE_T numBytesRead;
	HANDLE handle = GetCurrentProcess(); // no need to close

	char *p;
	if (bytes < sizeof(read_buffer))
		p = read_buffer;
	else
		p = malloc(bytes);
	BOOL success = ReadProcessMemory(handle, address + offset, p, bytes, &numBytesRead);
	if (!success)
	{
		if (p != read_buffer)
			free(p);
		return push_last_error(L, p_readError);
	}

	lua_pushlstring(L, p, numBytesRead);
	if (p != read_buffer)
		free(p);
	return 1;
}

int l_write(lua_State *L)
{
	char *address = lmr_to_pointer(L, L->base);
	SIZE_T offset = lmr_to_size_t(L, L->base + 1);
	address += offset;
	const TValue *o = L->base + 2;

	HANDLE handle = GetCurrentProcess();
	lmr_Value *u;
	size_t size;
	switch (o->tt)
	{
	case LUA_TBOOLEAN:
		if (!WriteProcessMemory(handle, address, &o->value.b, sizeof(BOOL), NULL))
			return push_last_error(L, p_writeError);
	case LUA_TNUMBER:
		if (!WriteProcessMemory(handle, address, &o->value.n, sizeof(lua_Number), NULL))
			return push_last_error(L, p_writeError);
	case LUA_TSTRING:
		if (!WriteProcessMemory(handle, address, svalue(o), tsvalue(o)->len, NULL))
			return push_last_error(L, p_writeError);
	case LUA_TUSERDATA:
		u = get_udata(o);
		switch (u->tt)
		{
		case LUA_TPOINTER:
			size = sizeof(LPVOID);
			break;
		case LUA_TUINT8:
		case LUA_TINT8:
			size = sizeof(UINT8);
			break;
		case LUA_TUINT16:
		case LUA_TINT16:
			size = sizeof(UINT16);
			break;
		case LUA_TUINT32:
		case LUA_TINT32:
			size = sizeof(UINT32);
			break;
		default:
			return luaL_error(L, p_typeError);
		}
		if (!WriteProcessMemory(handle, address, &u->uint32, size, NULL))
			return push_last_error(L, p_writeError);
	}
	return 0;
}

int process_modules_iterator(lua_State *L)
{
	/* stack = { nil (invariant state), module_t* (control variable) } */
	HANDLE *pUserData = (HANDLE *)lua_touserdata(L, lua_upvalueindex(1));
	HANDLE handle = *pUserData;
	static MODULEENTRY32 me32;
	me32.dwSize = sizeof(MODULEENTRY32);

	BOOL success;
	if (lua_isnil(L, 2)) // control variable is nil on first iteration
		success = Module32First(handle, &me32);
	else
		success = Module32Next(handle, &me32);

	if (!success)
		return 0;

	module_t *module = push_module(L); // control variable
	init_module(module, &me32);
	return 1;
}
int l_modules(lua_State *L)
{
	HANDLE *handlePtr = (HANDLE *)lua_newuserdata(L, sizeof(HANDLE *));
	luaL_getmetatable(L, SNAPSHOT_T);
	lua_setmetatable(L, -2);

	DWORD pid = GetCurrentProcessId();

	// From the WinAPI docs:
	// "If the function fails with ERROR_BAD_LENGTH when called with TH32CS_SNAPMODULE or TH32CS_SNAPMODULE32,
	// call the function again until it succeeds."
	do
	{
		*handlePtr = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
	} while (*handlePtr == INVALID_HANDLE_VALUE && GetLastError() == ERROR_BAD_LENGTH);

	if (*handlePtr == INVALID_HANDLE_VALUE)
	{
		// luaL_error(L, "Failed to create snapshot");
		return push_last_error(L, "failed to create snapshot");
	}

	// process_modules_iterator's upvalue is the HANDLE* userdata
	lua_pushcclosure(L, process_modules_iterator, 1);
	// we don't need invariant state variable
	return 1;
}

static const char *p_convertError = "attempt to convert %s into %s";
int l_pointer(lua_State *L)
{
	lmr_Value *u;
	const TValue *o = L->base; // index2adr(L, 1);
	switch (o->tt)
	{
	case LUA_TSTRING:
		LMR_PUSH_VALUE(L, LUA_TPOINTER, p, lmr_string_to_pointer(o));
		return 1;
	case LUA_TUSERDATA:
		u = get_udata(o);
		switch (u->tt)
		{
		case LUA_TPOINTER:
			lua_pushvalue(L, 1);
			return 1;
		}
		break;
	}
	return luaL_error(L, p_convertError, luaL_typename(L, 1), "pointer");
}
int l_uint8(lua_State *L)
{
	const TValue *o = L->base;
	switch (o->tt)
	{
	case LUA_TNUMBER:
		LMR_PUSH_VALUE(L, LUA_TUINT8, uint8, o->value.n);
		return 1;
	}
	return luaL_error(L, p_convertError, luaL_typename(L, 1), "uint8");
}
int l_int8(lua_State *L)
{
	const TValue *o = L->base;
	switch (o->tt)
	{
	case LUA_TNUMBER:
		LMR_PUSH_VALUE(L, LUA_TINT8, int8, o->value.n);
		return 1;
	}
	return luaL_error(L, p_convertError, luaL_typename(L, 1), "int8");
}
int l_uint16(lua_State *L)
{
	const TValue *o = L->base;
	switch (o->tt)
	{
	case LUA_TNUMBER:
		LMR_PUSH_VALUE(L, LUA_TUINT16, uint16, o->value.n);
		return 1;
	}
	return luaL_error(L, p_convertError, luaL_typename(L, 1), "uint16");
}
int l_int16(lua_State *L)
{
	const TValue *o = L->base;
	switch (o->tt)
	{
	case LUA_TNUMBER:
		LMR_PUSH_VALUE(L, LUA_TINT16, int16, o->value.n);
		return 1;
	}
	return luaL_error(L, p_convertError, luaL_typename(L, 1), "int16");
}
int l_uint32(lua_State *L)
{
	const TValue *o = L->base;
	switch (o->tt)
	{
	case LUA_TNUMBER:
		LMR_PUSH_VALUE(L, LUA_TUINT32, uint32, o->value.n);
		return 1;
	case LUA_TSTRING:
		LMR_PUSH_VALUE(L, LUA_TUINT32, uint32, *((UINT32 *)svalue(o)));
		return 1;
	}
	return luaL_error(L, p_convertError, luaL_typename(L, 1), "uint32");
}
int l_int32(lua_State *L)
{
	const TValue *o = L->base;
	switch (o->tt)
	{
	case LUA_TNUMBER:
		LMR_PUSH_VALUE(L, LUA_TINT32, int32, o->value.n);
		return 1;
	case LUA_TSTRING:
		LMR_PUSH_VALUE(L, LUA_TINT32, int32, *((INT32 *)svalue(o)));
		return 1;
	}
	return luaL_error(L, p_convertError, luaL_typename(L, 1), "int32");
}

int l_add(lua_State *L)
{
	static const char *pError = "attempt to perform addition operation on %s and %s";
	LMR_OPERATION_HEADER;

	switch (at)
	{
	case LUA_TNUMBER:
		LMR_aNUMBER_OPERATION_CASE_SWITCH(L, lua_pushnumber, +);
		break;
	case LUA_TUSERDATA:
		ua = get_udata(a);
		uc = lmr_push_userdata(L, ua->tt);
		switch (ua->tt)
		{
		case LUA_TPOINTER:
			switch (bt)
			{
			case LUA_TNUMBER:
				uc->p = (ua->p) + (UINT32)(b->value.n);
				return 1;
			case LUA_TSTRING:
				uc->p = (ua->p) + lmr_string_to_uint32(L, b);
				return 1;
			case LUA_TUSERDATA:
				ub = get_udata(b);
				switch (ub->tt)
				{
					LMR_OPERATION_CASE_LMR(L, p, ua->p, +)
				}
				break;
			};
			break;
			LMR_aUSERDATA_OPERATION_CASE_LMR(L, +)
		}
	}
	return luaL_error(L, pError, luaL_typename(L, 1), luaL_typename(L, 2));
}
int l_sub(lua_State *L)
{
	static const char *pError = "attempt to perform subtraction operation on %s and %s";
	LMR_OPERATION_HEADER;

	switch (at)
	{
	case LUA_TNUMBER:
		LMR_aNUMBER_OPERATION_CASE_SWITCH(L, lua_pushnumber, -);
		break;
	case LUA_TUSERDATA:
		ua = get_udata(a);
		uc = lmr_push_userdata(L, ua->tt);
		switch (ua->tt)
		{
		case LUA_TPOINTER:
			switch (bt)
			{
			case LUA_TNUMBER:
				uc->p = (ua->p) - (UINT32)(b->value.n);
				return 1;
			case LUA_TSTRING:
				uc->p = (ua->p) - lmr_string_to_uint32(L, b);
				return 1;
			case LUA_TUSERDATA:
				ub = get_udata(b);
				switch (ub->tt)
				{
				case LUA_TPOINTER:
					LMR_OPERATION(L, p, ua->p, p, -)
					return 1;
					LMR_OPERATION_CASE_LMR(L, p, ua->p, -)
				}
				break;
			};
			break;
			LMR_aUSERDATA_OPERATION_CASE_LMR(L, -)
		}
	}
	return luaL_error(L, pError, luaL_typename(L, 1), luaL_typename(L, 2));
}
int l_mult(lua_State *L)
{
	static const char *pError = "attempt to perform multiplication operation on %s and %s";
	LMR_OPERATION_HEADER;

	switch (at)
	{
	case LUA_TNUMBER:
		LMR_aNUMBER_OPERATION_CASE_SWITCH(L, lua_pushnumber, *);
		break;
	case LUA_TUSERDATA:
		ua = get_udata(a);
		uc = lmr_push_userdata(L, ua->tt);
		switch (ua->tt)
		{
			LMR_aUSERDATA_OPERATION_CASE_LMR(L, *)
		}
	}
	return luaL_error(L, pError, luaL_typename(L, 1), luaL_typename(L, 2));
}
int l_div(lua_State *L)
{
	static const char *pError = "attempt to perform divide operation on %s and %s";
	LMR_OPERATION_HEADER;
	ptrdiff_t ua_p;

	switch (at)
	{
	case LUA_TNUMBER:
		LMR_aNUMBER_OPERATION_CASE_SWITCH(L, lua_pushnumber, /);
		break;
	case LUA_TUSERDATA:
		ua = get_udata(a);
		uc = lmr_push_userdata(L, ua->tt);
		switch (ua->tt)
		{
		case LUA_TPOINTER:
			switch (bt)
			{
			case LUA_TNUMBER:
				uc->p = ((ptrdiff_t)ua->p) / (UINT32)(b->value.n);
				return 1;
			case LUA_TSTRING:
				uc->p = ((ptrdiff_t)ua->p) / lmr_string_to_uint32(L, b);
				return 1;
			case LUA_TUSERDATA:
				ub = get_udata(b);
				ua_p = (ptrdiff_t)ua->p;
				switch (ub->tt)
				{
					LMR_OPERATION_CASE_LMR(L, p, ua_p, /)
				}
				break;
			};
			break;
			LMR_aUSERDATA_OPERATION_CASE_LMR(L, /)
		}
	}
	return luaL_error(L, pError, luaL_typename(L, 1), luaL_typename(L, 2));
}

static const char *p_operationError = "attempt to perform %s operation on %s and %s";
int l_gt(lua_State *L)
{
	LMR_TEST_HEADER;
	LMR_aTEST(L, >);
	return luaL_error(L, p_operationError, ">", luaL_typename(L, 1), luaL_typename(L, 2));
}
int l_lt(lua_State *L)
{
	LMR_TEST_HEADER;
	LMR_aTEST(L, <);
	return luaL_error(L, p_operationError, "<", luaL_typename(L, 1), luaL_typename(L, 2));
}
int l_eq(lua_State *L)
{
	LMR_TEST_HEADER;
	LMR_aTEST(L, ==);
	return luaL_error(L, p_operationError, "==", luaL_typename(L, 1), luaL_typename(L, 2));
}

int l_type(lua_State *L)
{
	lmr_Value *uo;
	const TValue *o = L->base;
	switch (o->tt)
	{
	case LUA_TNIL:
		lua_pushliteral(L, "nil");
		return 1;
	case LUA_TBOOLEAN:
		lua_pushliteral(L, "boolean");
		return 1;
	case LUA_TNUMBER:
		lua_pushliteral(L, "float");
		return 1;
	case LUA_TSTRING:
		lua_pushliteral(L, "bytes");
		return 1;
	case LUA_TUSERDATA:
		uo = get_udata(o);
		switch (uo->tt)
		{
		case LUA_TPOINTER:
			lua_pushliteral(L, "pointer");
			return 1;
		case LUA_TUINT8:
			lua_pushliteral(L, "uint8");
			return 1;
		case LUA_TINT8:
			lua_pushliteral(L, "int8");
			return 1;
		case LUA_TUINT16:
			lua_pushliteral(L, "uint16");
			return 1;
		case LUA_TINT16:
			lua_pushliteral(L, "int16");
			return 1;
		case LUA_TUINT32:
			lua_pushliteral(L, "uint32");
			return 1;
		case LUA_TINT32:
			lua_pushliteral(L, "int32");
			return 1;
		}
	}
	return luaL_error(L, "invalid type %s", luaL_typename(L, 1));
}
int l_tostring(lua_State *L)
{
	const char *p;
	size_t tl;
	char *tmp;
	size_t i;
	lmr_Value *uo;
	const TValue *o = L->base; // index2adr(L, 1)
	switch (o->tt)
	{
	case LUA_TNIL:
		lua_pushliteral(L, "nil");
		return 1;
	case LUA_TBOOLEAN:
		lua_pushstring(L, (o->value.b ? "true" : "false")); // lua_toboolean(L, 1)
		return 1;
	case LUA_TNUMBER:
		lua_pushstring(L, lua_tostring(L, 1));
		return 1;
	case LUA_TSTRING:
		p = svalue(o);
		tl = tsvalue(o)->len;
		if (tl == 0)
		{
			lua_pushvalue(L, 1);
			return 1;
		}
		if (tl * 2 < sizeof(read_buffer))
			tmp = read_buffer;
		else
			tmp = malloc(tl * 2);
		static const char hex_digits[] = "0123456789ABCDEF";
		for (i = 0; i < tl; ++i)
		{
			unsigned char ch = p[i];
			tmp[i * 2] = hex_digits[ch >> 4];
			tmp[i * 2 + 1] = hex_digits[ch & 15];
		}
		lua_pushlstring(L, tmp, tl * 2);
		if (tmp != read_buffer)
			free(tmp);
		return 1;
	case LUA_TUSERDATA:
		uo = get_udata(o);
		switch (uo->tt)
		{
		case LUA_TPOINTER:
			lua_pushfstring(L, "%p", uo->p);
			return 1;
		case LUA_TUINT8:
			sprintf_s(read_buffer, sizeof(read_buffer), "%hhu", uo->uint8);
			break;
		case LUA_TINT8:
			sprintf_s(read_buffer, sizeof(read_buffer), "%hhd", uo->int8);
			break;
		case LUA_TUINT16:
			sprintf_s(read_buffer, sizeof(read_buffer), "%hu", uo->uint16);
			break;
		case LUA_TINT16:
			sprintf_s(read_buffer, sizeof(read_buffer), "%hd", uo->int16);
			break;
		case LUA_TUINT32:
			sprintf_s(read_buffer, sizeof(read_buffer), "%u", uo->uint32);
			break;
		case LUA_TINT32:
			sprintf_s(read_buffer, sizeof(read_buffer), "%d", uo->int32);
			break;
		}
		lua_pushstring(L, read_buffer);
		return 1;
	}
	return luaL_error(L, "attempt to print %s as special type", luaL_typename(L, 1));
}
int l_tonumber(lua_State *L)
{
	size_t num = lmr_to_size_t(L, L->base);
	lua_pushnumber(L, num);
	return 1;
}

int l_ud_topointer(lua_State *L)
{
	const TValue *o = L->base;
	lmr_push_userdata(L, LUA_TPOINTER)->p = *(intptr_t *)get_udata(o);
	return 1;
}
int l_ud_debug(lua_State *L)
{
	const TValue *o = L->base;
	lua_pushnumber(L, o->tt);
	lmr_push_userdata(L, LUA_TPOINTER)->p = o->value.p;
	return 2;
}

int l_createtable(lua_State *L)
{
	size_t narr = lmr_to_size_t(L, L->base);
	size_t nrec = lmr_to_size_t(L, L->base + 1);
	lua_createtable(L, narr, nrec);
	return 1;
}

const luaL_Reg lib[] = {
	{"read_float", l_read_float},
	{"read_pointer", l_read_pointer},
	{"read_uint8", l_read_uint8},
	{"read_int8", l_read_int8},
	{"read_uint16", l_read_uint16},
	{"read_int16", l_read_int16},
	{"read_uint32", l_read_uint32},
	{"read_int32", l_read_int32},

	{"read_boolean", l_read_boolean},
	{"read_string", l_read_string},
	{"read_array", l_read_array},
	{"read_rowidx", l_read_rowidx},
	{"read", l_read},
	{"write", l_write},

	{"modules", l_modules},

	{"pointer", l_pointer},
	{"uint8", l_uint8},
	{"int8", l_int8},
	{"uint16", l_uint16},
	{"int16", l_int16},
	{"uint32", l_uint32},
	{"int32", l_int32},

	{"add", l_add},
	{"sub", l_sub},
	{"mult", l_mult},
	{"div", l_div},

	{"gt", l_gt},
	{"lt", l_lt},
	{"eq", l_eq},

	{"type", l_type},
	{"tostring", l_tostring},
	{"tonumber", l_tonumber},
	{"ud_topointer", l_ud_topointer},
	{"ud_debug", l_ud_debug},

	{"createtable", l_createtable},
	{NULL, NULL}};

EXPORT int __cdecl luaopen_memreader(lua_State *L)
{
	init_lua_ptr(L);

	// luaL_newlib(L, lib);
	luaL_newlibtable(L, lib);
	luaL_register(L, NULL, lib);
	// luaL_register(L, "memreader", lib);

	lua_pushliteral(L, "base");
	lmr_push_userdata(L, LUA_TPOINTER)->p = GetModuleHandleA(NULL);
	lua_settable(L, -3);
	lua_pushliteral(L, "version");
	lua_pushnumber(L, APP_VERSION);
	lua_settable(L, -3);
	// lua_pushliteral(L, "lpvoid");
	// lua_pushnumber(L, sizeof(LPVOID));
	// lua_settable(L, -3);

	register_module(L);
	register_snapshot(L);

	return 1;
}

// ucrt.lib
// libcmt.lib
// libvcruntime.lib
// BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
// //   printf("load/unload my DLL\n");
//   return TRUE;
// }