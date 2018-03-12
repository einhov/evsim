#include <cstdio>
#include <stdexcept>
#include <string_view>
#include <optional>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "lua_conf.h"

namespace evsim {

lua_conf::lua_conf(const std::string_view &name, int argc, char **argv) {
	L = luaL_newstate();
	luaL_openlibs(L);
	lua_createtable(L, argc, argc);
	for(lua_Integer n = 0; n < argc; n++) {
		lua_pushstring(L, argv[n]);
		lua_seti(L, -2, n);
	}
	lua_setglobal(L, "arg");

	if(name.empty()) {
		fprintf(stderr, "Warning: Running without configuration file\n");
		return;
	}

	if(luaL_dofile(L, name.data())) {
		fprintf(stderr, "Error: lua_conf dofile error: %s. Continuing without configuration file\n", lua_tostring(L, -1));
		lua_pop(L, 1);
	}
}

lua_conf::~lua_conf() {
	lua_close(L);
}

bool lua_conf::enter_table(const std::string_view &name, bool global) {
	if(global)
		lua_getglobal(L, name.data());
	else
		lua_getfield(L, -1, name.data());

	if(!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return false;
	} else {
		return true;
	}
}

void lua_conf::enter_table_or_empty(const std::string_view &name, bool global) {
	if(!enter_table(name, global))
		enter_empty_table();
}

void lua_conf::enter_empty_table() {
	lua_newtable(L);
}

void lua_conf::leave_table() {
	lua_pop(L, 1);
}

std::optional<lua_Integer> lua_conf::get_integer(const std::string_view &name, bool global) {
	if(global)
		lua_getglobal(L, name.data());
	else
		lua_getfield(L, -1, name.data());

	if(lua_isinteger(L, -1)) {
		const auto result = lua_tointeger(L, -1);
		lua_pop(L, 1);
		return result;
	} else {
		const auto type = lua_type(L, -1);
		if(type != LUA_TNIL) {
			fprintf(
				stderr,
				"lua_conf::get_integer: %s is a %s\n",
				name.data(), lua_typename(L, type)
			);
		}
		lua_pop(L, 1);
		return {};
	}
}

std::optional<lua_Number> lua_conf::get_number(const std::string_view &name, bool global) {
	if(global)
		lua_getglobal(L, name.data());
	else
		lua_getfield(L, -1, name.data());

	if(lua_isnumber(L, -1)) {
		const auto result = lua_tonumber(L, -1);
		lua_pop(L, 1);
		return result;
	} else {
		const auto type = lua_type(L, -1);
		if(type != LUA_TNIL) {
			fprintf(
				stderr,
				"lua_conf::get_number: %s is a %s\n",
				name.data(), lua_typename(L, type)
			);
		}
		lua_pop(L, 1);
		return {};
	}
}

std::optional<std::string> lua_conf::get_string(const std::string_view &name, bool global) {
	if(global)
		lua_getglobal(L, name.data());
	else
		lua_getfield(L, -1, name.data());

	if(lua_isstring(L, -1)) {
		const auto result = std::string(lua_tostring(L, -1));
		lua_pop(L, 1);
		return result;
	} else {
		const auto type = lua_type(L, -1);
		if(type != LUA_TNIL) {
			fprintf(
				stderr,
				"lua_conf::get_string: %s is a %s\n",
				name.data(), lua_typename(L, type)
			);
		}
		lua_pop(L, 1);
		return {};
	}
}

std::optional<bool> lua_conf::get_boolean(const std::string_view &name, bool global) {
	if(global)
		lua_getglobal(L, name.data());
	else
		lua_getfield(L, -1, name.data());

	if(lua_isboolean(L, -1)) {
		const auto result = lua_toboolean(L, -1);
		lua_pop(L, 1);
		return result;
	} else {
		const auto type = lua_type(L, -1);
		if(type != LUA_TNIL) {
			fprintf(
				stderr,
				"lua_conf::get_boolean: %s is a %s\n",
				name.data(), lua_typename(L, type)
			);
		}
		lua_pop(L, 1);
		return {};
	}
}

lua_Integer lua_conf::get_integer_default(const std::string_view &name, lua_Integer _default, bool global) {
	const auto value = get_integer(name, global);
	return value ? *value : _default;
}

lua_Number lua_conf::get_number_default(const std::string_view &name, lua_Number _default, bool global) {
	const auto value = get_number(name, global);
	return value ? *value : _default;
}

std::string lua_conf::get_string_default(const std::string_view &name, const std::string_view &_default, bool global) {
	const auto value = get_string(name, global);
	return value ? std::move(*value) : std::string(_default);
}

bool lua_conf::get_boolean_default(const std::string_view &name, bool _default, bool global) {
	const auto value = get_boolean(name, global);
	return value ? *value : _default;
}

}
