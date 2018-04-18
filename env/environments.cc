#include <memory>
#include <string_view>
#include <unordered_map>

#include "environments.h"
#include "../lua_conf.h"

namespace evsim {

static std::unique_ptr<environment_base> make_food_env(lua_conf &conf) {
	auto result = std::make_unique<food::environment>();
	result->init(conf);
	return result;
}

static std::unique_ptr<environment_base> make_multi_food_env(lua_conf &conf) {
	auto result = std::make_unique<multi_food::environment>();
	result->init(conf);
	return result;
}

static std::unique_ptr<environment_base> make_multi_move_env(lua_conf &conf) {
	auto result = std::make_unique<multi_move::environment>();
	result->init(conf);
	return result;
}

static std::unique_ptr<environment_base> make_door_env(lua_conf &conf) {
	auto result = std::make_unique<door::environment>();
	result->init(conf);
	return result;
}

using namespace std::literals::string_view_literals;
using factory_function = std::unique_ptr<environment_base> (*)(lua_conf&);
static const std::unordered_map<std::string_view, factory_function> factory_by_name {
	{ "food"sv, make_food_env },
	{ "multi_food"sv, make_multi_food_env },
	{ "multi_move"sv, make_multi_move_env },
	{ "door"sv, make_door_env }
};

std::unique_ptr<environment_base> make_environment(lua_conf &conf) {
	std::unique_ptr<environment_base> result;

	if(conf.enter_table("environment", true)) {
		const auto name = conf.get_string_default("name", "");
		const auto factory = factory_by_name.find(name);
		if(factory != factory_by_name.cend()) {
			result = factory->second(conf);
		}
		conf.leave_table();
	} else {
		conf.enter_empty_table();
		result = make_food_env(conf);
	}

	return result;
}

}
