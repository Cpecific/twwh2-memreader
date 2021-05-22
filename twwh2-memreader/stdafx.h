#pragma once

#ifdef _WIN32
#undef UNICODE
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#define PSAPI_VERSION 1
#include <windows.h>
#endif

#define MEMREADER_METATABLE(name) "memreader." #name
// #define GetCurrentProcess() ((HANDLE)-1)

#include "config.h"

#if defined(_MSC_VER)
//  Microsoft
#define EXPORT __declspec(dllexport)
#define IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
//  GCC
#define EXPORT __attribute__((visibility("default")))
#define IMPORT
#else
#pragma error "Unknown dynamic link import/export semantics."
#endif