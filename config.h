#ifndef CONFIG_H
#define CONFIG_H

#include <cstddef>
#include "lua_conf.h"

namespace evsim {
namespace build_config {

extern size_t food_count;
extern size_t herbivore_count;
extern size_t predator_count;

// Physics
extern float linear_damping;
extern float angular_damping;

// Hervivores
extern size_t hv_min_species;
extern size_t hv_max_species;
extern double hv_compat_treshold;
extern float hv_force;
extern float hv_torque;

// Predator
extern size_t pr_min_species;
extern size_t pr_max_species;
extern double pr_compat_treshold;
extern float pr_force;
extern float pr_torque;

// Sensors
extern float sensor_length;
extern float sensor_fov;

void load_config(lua_conf &conf);

}
}

#endif
