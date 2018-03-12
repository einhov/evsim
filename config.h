#ifndef CONFIG_H
#define CONFIG_H

#include <glm/glm.hpp>
#include <cstddef>
#include "lua_conf.h"

namespace evsim::config {

// Physics
inline float linear_damping = 10.0f;
inline float angular_damping = 10.0f;

// Sensors
inline float sensor_length = 45.0f;
inline float sensor_fov = glm::radians(60.0f);

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
}

}

#endif
