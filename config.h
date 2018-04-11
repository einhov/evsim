#ifndef CONFIG_H
#define CONFIG_H

#include <optional>
#include <cstddef>

#include <glm/glm.hpp>

#include "lua_conf.h"

namespace evsim::config {

// Physics
inline float linear_damping = 10.0f;
inline float angular_damping = 10.0f;

// Sensors
inline float sensor_length = 45.0f;
inline float sensor_fov = glm::radians(60.0f);

inline std::optional<size_t> max_generations;

inline void load_config(lua_conf &conf) {
	if(conf.enter_table("physics", true)) {
		linear_damping = conf.get_number_default("linear_damping", 10.0);
		angular_damping = conf.get_number_default("angular_damping", 10.0);
	}

	if(conf.enter_table("sensors", true)) {
		sensor_length = conf.get_number_default("length", 45.0);
		sensor_fov = glm::radians(conf.get_number_default("fov", 60.0));
		conf.leave_table();
	}

	if(conf.enter_table("_G", true)) {
		if(const auto max = conf.get_integer("max_generations", true); max && (*max) > 0)
			max_generations = max;
	}
}

}

#endif
