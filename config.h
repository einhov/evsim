#ifndef CONFIG_H
#define CONFIG_H

#include <glm/glm.hpp>

namespace evsim {
namespace build_config {

constexpr size_t food_count = 150;
constexpr size_t herbivore_count = 50;
constexpr size_t predator_count = 50;

// Physics
constexpr float linear_damping = 10.0f;
constexpr float angular_damping = 10.0f;

// Hervivores
constexpr size_t hv_min_species = 3;
constexpr size_t hv_max_species = 20;
constexpr double hv_compat_treshold = 0.1;
constexpr float hv_force = 1000.0f;
constexpr float hv_torque = 45.0f;

// Predator
constexpr size_t pr_min_species = 3;
constexpr size_t pr_max_species = 20;
constexpr double pr_compat_treshold = 0.1;
constexpr float pr_force = 1000.0f;
constexpr float pr_torque = 75.0f;

// Sensors
constexpr float sensor_length = 45.0f;
constexpr float sensor_fov = glm::radians(60.0f);

}
}

#endif
