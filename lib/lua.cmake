PROJECT(lua)
cmake_minimum_required(VERSION 2.8)

set(lua_SOURCE_DIR "${PROJECT_SOURCE_DIR}/lib/lua-5.3.4")

set(lua_SOURCES
	"${lua_SOURCE_DIR}/src/lobject.c"
	"${lua_SOURCE_DIR}/src/loadlib.c"
	"${lua_SOURCE_DIR}/src/lvm.c"
	"${lua_SOURCE_DIR}/src/lstrlib.c"
	"${lua_SOURCE_DIR}/src/lbitlib.c"
	"${lua_SOURCE_DIR}/src/lapi.c"
	"${lua_SOURCE_DIR}/src/luac.c"
	"${lua_SOURCE_DIR}/src/ldebug.c"
	"${lua_SOURCE_DIR}/src/linit.c"
	"${lua_SOURCE_DIR}/src/lcorolib.c"
	"${lua_SOURCE_DIR}/src/lzio.c"
	"${lua_SOURCE_DIR}/src/lmathlib.c"
	"${lua_SOURCE_DIR}/src/lgc.c"
	"${lua_SOURCE_DIR}/src/ltable.c"
	"${lua_SOURCE_DIR}/src/lutf8lib.c"
	"${lua_SOURCE_DIR}/src/lua.c"
	"${lua_SOURCE_DIR}/src/lstate.c"
	"${lua_SOURCE_DIR}/src/lmem.c"
	"${lua_SOURCE_DIR}/src/ldblib.c"
	"${lua_SOURCE_DIR}/src/lfunc.c"
	"${lua_SOURCE_DIR}/src/lauxlib.c"
	"${lua_SOURCE_DIR}/src/lstring.c"
	"${lua_SOURCE_DIR}/src/ltm.c"
	"${lua_SOURCE_DIR}/src/lbaselib.c"
	"${lua_SOURCE_DIR}/src/ldump.c"
	"${lua_SOURCE_DIR}/src/lopcodes.c"
	"${lua_SOURCE_DIR}/src/lcode.c"
	"${lua_SOURCE_DIR}/src/lundump.c"
	"${lua_SOURCE_DIR}/src/lctype.c"
	"${lua_SOURCE_DIR}/src/ldo.c"
	"${lua_SOURCE_DIR}/src/lparser.c"
	"${lua_SOURCE_DIR}/src/loslib.c"
	"${lua_SOURCE_DIR}/src/liolib.c"
	"${lua_SOURCE_DIR}/src/llex.c"
	"${lua_SOURCE_DIR}/src/ltablib.c"
)

set_source_files_properties(${lua_SOURCES} PROPERTIES LANGUAGE CXX)

add_library(lua STATIC ${lua_SOURCES})
target_include_directories(lua PUBLIC ${lua_SOURCE_DIR}/src)
target_compile_options(lua PRIVATE -Wall)
target_compile_definitions(lua PRIVATE -DLUA_USE_POSIX)
