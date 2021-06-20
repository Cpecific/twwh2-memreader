cmake_minimum_required (VERSION 3.8)

project ("twwh2_memreader" VERSION 1.1)

add_library(twwh2_memreader SHARED
	"stdafx.h"
	"memreader.c" "memreader.h"
	"lua_ptr.c" "lua_ptr.h"
	"module.c" "module.h"
	"utils.c" "utils.h")

configure_file(config.h.in config.h)

target_include_directories(twwh2_memreader PUBLIC
	"${PROJECT_BINARY_DIR}"
	"./lua-5.1/src")

IF(WIN32)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /GS /TC /GL /Gy /Zi /Gm- /O1 /sdl /fp:precise /Gd /Oy- /Oi /MD /FC /Os")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /MANIFEST:NO /LTCG /NOENTRY /NXCOMPAT /DYNAMICBASE:NO /LARGEADDRESSAWARE /MANIFESTUAC:NO /NODEFAULTLIB")
	target_link_libraries(twwh2_memreader ucrt.lib libcmt.lib libvcruntime.lib)
ENDIF(WIN32)