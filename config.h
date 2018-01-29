#ifndef CONFIG_H
#define CONFIG_H

namespace evsim {
namespace build_config {

constexpr size_t FOODS = 150;
constexpr size_t herbivores_size = 50;
constexpr size_t predator_size = 50;

//Physics
constexpr float linear_damping = 25.0f;
constexpr float angular_damping = 25.0f;

//Hervivores
constexpr size_t hv_min_species = 3;
constexpr size_t hv_max_species = 20;
constexpr bool hv_dont_use_bias_neuron = false;
constexpr double hv_compat_treshold = 0.1;
constexpr float hv_linear_speed = 700.0f;
constexpr float hv_angular_speed = 45.0f;

//Predator
constexpr size_t pr_min_species = 3;
constexpr size_t pr_max_species = 20;
constexpr bool pr_dont_use_bias_neuron = false;
constexpr double pr_compat_treshold = 0.1;
constexpr float pr_linear_speed = 1500.0f;
constexpr float pr_angular_speed = 150.0f;

//Sensors
constexpr float sensor_length = 75.0f;
constexpr float sensor_half_width = 10.0f;

}
}
#endif
