#include <glm/glm.hpp>

#include "config.h"
#include "lua_conf.h"

namespace evsim {
namespace build_config {

size_t food_count = 150;
size_t herbivore_count = 150;
size_t predator_count = 50;

// Physics
float linear_damping = 10.0f;
float angular_damping = 10.0f;

// Hervivores
size_t hv_min_species = 3;
size_t hv_max_species = 20;
double hv_compat_treshold = 5.0;
float hv_force = 1000.0f;
float hv_torque = 45.0f;

// Predator
size_t pr_min_species = 3;
size_t pr_max_species = 20;
double pr_compat_treshold = 0.1;
float pr_force = 1000.0f;
float pr_torque = 45.0f;

// Sensors
float sensor_length = 45.0f;
float sensor_fov = glm::radians(60.0f);

void load_config(lua_conf &conf) {
	build_config::food_count = conf.get_integer_default("food_count", 150, true);;

	if(conf.enter_table("herbivores", true)) {
		build_config::herbivore_count = conf.get_integer_default("count", 150);
		build_config::hv_min_species = conf.get_integer_default("min_species", 3);
		build_config::hv_max_species = conf.get_integer_default("max_species", 20);
		build_config::hv_compat_treshold = conf.get_number_default("compat_thresh", 5.0);
		build_config::hv_force = conf.get_number_default("force", 1000.0);
		build_config::hv_torque = conf.get_number_default("torque", 45.0);
		conf.leave_table();
	}

	if(conf.enter_table("predators", true)) {
		build_config::predator_count = conf.get_integer_default("count", 50);
		build_config::pr_min_species = conf.get_integer_default("min_species", 3);
		build_config::pr_max_species = conf.get_integer_default("max_species", 20);
		build_config::pr_compat_treshold = conf.get_number_default("compat_thresh", 5.0);
		build_config::pr_force = conf.get_number_default("force", 1000.0);
		build_config::pr_torque = conf.get_number_default("torque", 45.0);
		conf.leave_table();
	}

	if(conf.enter_table("sensors", true)) {
		build_config::sensor_length = conf.get_number_default("length", 45.0);
		build_config::sensor_fov = glm::radians(conf.get_number_default("fov", 60.0));
		conf.leave_table();
	}
}

}
}
