#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#include "../../environment_base.h"
#include "../../species.h"
#include "../../evsim.h"
#include "../../config.h"
#include "../../consumable.h"

#include "environment.h"
#include "herbivore_neat.h"
#include "predator_neat.h"

namespace evsim {
namespace multi_food {

environment::environment() : herbivores(*conf.world), predator(*conf.world) {}

void environment::init() {
	herbivores.initialise(build_config::herbivore_count, static_cast<int>(glfwGetTime()));
	predator.initialise(build_config::predator_count, static_cast<int>(glfwGetTime()+1));

	for(int i = 0; i < build_config::food_count; i++) {
		auto consumable_instance = std::make_unique<consumable>();
		consumable_instance->init_body((*conf.world));
		environmental_objects.emplace_back(std::move(consumable_instance));
	}
}

void environment::step() {
	herbivores.step();
	predator.step();
}

void environment::epoch() {
	herbivores.epoch(STEPS_PER_GENERATION);
	predator.epoch(STEPS_PER_GENERATION);
}

void environment::pre_tick() {
	herbivores.pre_tick();
	predator.pre_tick();
}

void environment::tick() {
	herbivores.tick();
	predator.tick();
	for(auto &env_obj : environmental_objects) {
		env_obj->tick();
	}
}

void environment::draw() {
	// Draw environmental objects
	for(const auto &env_obj : environmental_objects){
		env_obj->draw(conf.projection);
	}
	//draw agents
	predator.draw(conf.projection);
	herbivores.draw(conf.projection);
}

}
}
