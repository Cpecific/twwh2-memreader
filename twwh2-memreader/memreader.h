#pragma once

#include "stdafx.h"
#include "lua_ptr.h"

#define APP_VERSION (twwh2_memreader_VERSION_MAJOR.twwh2_memreader_VERSION_MINOR)
#define SNAPSHOT_T MEMREADER_METATABLE(snapshot)

#define LMR_OPERATION(L, _field, _a, _bfield, operation) uc->_field = (_a)##operation##(ub->_bfield);

/* at = TUSERDATA; ua->tt != TPOINTER; bt = TUSERDATA */
#define LMR_OPERATION_CASE_LMR(L, _field, _a, operation) \
	case LUA_TUINT8:                                     \
		LMR_OPERATION(L, _field, _a, uint8, operation)   \
		return 1;                                        \
	case LUA_TINT8:                                      \
		LMR_OPERATION(L, _field, _a, int8, operation)    \
		return 1;                                        \
	case LUA_TUINT16:                                    \
		LMR_OPERATION(L, _field, _a, uint16, operation)  \
		return 1;                                        \
	case LUA_TINT16:                                     \
		LMR_OPERATION(L, _field, _a, int16, operation)   \
		return 1;                                        \
	case LUA_TUINT32:                                    \
		LMR_OPERATION(L, _field, _a, uint32, operation)  \
		return 1;                                        \
	case LUA_TINT32:                                     \
		LMR_OPERATION(L, _field, _a, int32, operation)   \
		return 1;

/* at = TUSERDATA; ua->tt != TPOINTER */
#define LMR_OPERATION_CASE_SWITCH(L, _field, operation)              \
	switch (bt)                                                      \
	{                                                                \
	case LUA_TNUMBER:                                                \
		uc->_field = (ua->_field)##operation##(UINT32)(b->value.n);  \
		return 1;                                                    \
	case LUA_TSTRING:                                                \
		uc->p = (ua->_field)##operation##(lmr_string_to_uint32(b));  \
		return 1;                                                    \
	case LUA_TUSERDATA:                                              \
		ub = get_udata(b);                                           \
		switch (ub->tt)                                              \
		{                                                            \
			LMR_OPERATION_CASE_LMR(L, _field, ua->_field, operation) \
		}                                                            \
	};

#define LMR_aNUMBER_OPERATION_CASE_SWITCH(L, func, operation) \
	switch (bt)                                               \
	{                                                         \
	case LUA_TNUMBER:                                         \
		func(L, (a->value.n)##operation##(b->value.n));       \
		return 1;                                             \
	case LUA_TUSERDATA:                                       \
		ub = get_udata(b);                                    \
		switch (ub->tt)                                       \
		{                                                     \
		case LUA_TUINT8:                                      \
			func(L, (a->value.n)##operation##(ub->uint8));    \
			return 1;                                         \
		case LUA_TINT8:                                       \
			func(L, (a->value.n)##operation##(ub->int8));     \
			return 1;                                         \
		case LUA_TUINT16:                                     \
			func(L, (a->value.n)##operation##(ub->uint16));   \
			return 1;                                         \
		case LUA_TINT16:                                      \
			func(L, (a->value.n)##operation##(ub->int16));    \
			return 1;                                         \
		case LUA_TUINT32:                                     \
			func(L, (a->value.n)##operation##(ub->uint32));   \
			return 1;                                         \
		case LUA_TINT32:                                      \
			func(L, (a->value.n)##operation##(ub->int32));    \
			return 1;                                         \
		}                                                     \
	}

#define LMR_aUSERDATA_OPERATION_CASE_LMR(L, operation)  \
	case LUA_TUINT8:                                    \
		LMR_OPERATION_CASE_SWITCH(L, uint8, operation)  \
		break;                                          \
	case LUA_TINT8:                                     \
		LMR_OPERATION_CASE_SWITCH(L, int8, operation)   \
		break;                                          \
	case LUA_TUINT16:                                   \
		LMR_OPERATION_CASE_SWITCH(L, uint16, operation) \
		break;                                          \
	case LUA_TINT16:                                    \
		LMR_OPERATION_CASE_SWITCH(L, int16, operation)  \
		break;                                          \
	case LUA_TUINT32:                                   \
		LMR_OPERATION_CASE_SWITCH(L, uint32, operation) \
		break;                                          \
	case LUA_TINT32:                                    \
		LMR_OPERATION_CASE_SWITCH(L, int32, operation)  \
		break;

#define LMR_OPERATION_HEADER       \
	lmr_Value *ua, *ub, *uc;       \
	const TValue *a = L->base;     \
	int at = a->tt;                \
	const TValue *b = L->base + 1; \
	int bt = b->tt;

// ! TEST
#define LMR_TEST(L, _a, _bfield, operation) lua_pushboolean(L, (_a)##operation##(ub->_bfield));

/* at = TUSERDATA; ua->tt == [TPOINTER, TU?INT32]; */
#define LMR_TEST_CASE_SWITCH(L, _field, operation)                          \
	_field = ua->_field;                                                    \
	switch (bt)                                                             \
	{                                                                       \
	case LUA_TNUMBER:                                                       \
		lua_pushboolean(L, (_field)##operation##(UINT32)(b->value.n));      \
		return 1;                                                           \
	case LUA_TSTRING:                                                       \
		lua_pushboolean(L, (_field)##operation##(lmr_string_to_uint32(b))); \
		return 1;                                                           \
	case LUA_TUSERDATA:                                                     \
		ub = get_udata(b);                                                  \
		switch (ub->tt)                                                     \
		{                                                                   \
		case LUA_TUINT32:                                                   \
			LMR_TEST(L, _field, uint32, operation)                          \
			return 1;                                                       \
		case LUA_TINT32:                                                    \
			LMR_TEST(L, _field, int32, operation)                           \
			return 1;                                                       \
		}                                                                   \
	};

#define LMR_aUSERDATA_TEST_CASE_LMR(L, operation)                              \
	case LUA_TPOINTER:                                                         \
		p = ua->p;                                                             \
		switch (bt)                                                            \
		{                                                                      \
		case LUA_TSTRING:                                                      \
			lua_pushboolean(L, (p)##operation##(lmr_string_to_pointer(b))); \
			return 1;                                                          \
		case LUA_TUSERDATA:                                                    \
			ub = get_udata(b);                                                 \
			switch (ub->tt)                                                    \
			{                                                                  \
			case LUA_TPOINTER:                                                 \
				LMR_TEST(L, p, p, operation)                                   \
				return 1;                                                      \
			}                                                                  \
		};                                                                     \
		break;                                                                 \
	case LUA_TUINT32:                                                          \
		LMR_TEST_CASE_SWITCH(L, uint32, operation)                             \
		break;                                                                 \
	case LUA_TINT32:                                                           \
		LMR_TEST_CASE_SWITCH(L, int32, operation)                              \
		break;

#define LMR_aTEST(L, operation)                       \
	switch (at)                                       \
	{                                                 \
	case LUA_TUSERDATA:                               \
		ua = get_udata(a);                            \
		switch (ua->tt)                               \
		{                                             \
			LMR_aUSERDATA_TEST_CASE_LMR(L, operation) \
		}                                             \
	}

#define LMR_TEST_HEADER            \
	lmr_Value *ua, *ub, *uc;       \
	const TValue *a = L->base;     \
	int at = a->tt;                \
	const TValue *b = L->base + 1; \
	int bt = b->tt;                \
	INT_PTR p;                     \
	UINT32 uint32;                 \
	INT32 int32;

// LMR_PVALUE(_type, i_o) = (_type)_value;
#define LMR_SET_VALUE(L, p_udata, _tt, _field, _value) \
	p_udata->tt = _tt;                                 \
	p_udata->_field = _value;

#define LMR_PUSH_VALUE(L, _tt, _field, _value) \
	lmr_push_userdata(L, _tt)->_field = _value;

lmr_Value *lmr_push_userdata(lua_State *L, BYTE tt);