#ifndef LUA_CONF_H
#define LUA_CONF_H

#include <string_view>
#include <optional>

#include <lua.h>

namespace evsim {

class lua_conf {
	public:
		lua_conf(const std::string_view &name, int argc, char **argv);
		~lua_conf();

		bool enter_table(const std::string_view &name, bool global = false);
		void leave_table();

		std::optional<lua_Integer> get_integer(const std::string_view &name, bool global = false);
		std::optional<lua_Number> get_number(const std::string_view &name, bool global = false);
		std::optional<std::string> get_string(const std::string_view &name, bool global = false);
		std::optional<bool> get_boolean(const std::string_view &name, bool global = false);

		lua_Integer get_integer_default(const std::string_view &name, lua_Integer _default, bool global = false);
		lua_Number get_number_default(const std::string_view &name, lua_Number _default, bool global = false);
		std::string get_string_default(const std::string_view &name, const std::string_view &_default, bool global = false);
		bool get_boolean_default(const std::string_view &name, bool _default, bool global = false);

	private:
		lua_State *L;
};

}

#endif
