#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#include "env_food.h"
#include "environment.h"
#include "species.h"
#include "species_neat.h"
#include "predator_neat.h"
#include "evsim.h"
#include "config.h"
#include "consumable.h"

namespace evsim {

env_food::env_food() : herbivores(*conf.world), predator(*conf.world) {}

void env_food::init() {
	herbivores.initialise(build_config::herbivore_count, static_cast<int>(glfwGetTime()));
	predator.initialise(build_config::predator_count, static_cast<int>(glfwGetTime()+1));

	for(int i = 0; i < build_config::food_count; i++) {
		auto consumable_instance = std::make_unique<consumable>();
		consumable_instance->init_body((*conf.world));
		environmental_objects.emplace_back(std::move(consumable_instance));
	}
}

void env_food::step() {
	herbivores.step();
	predator.step();
}

void env_food::epoch() {
	herbivores.epoch(STEPS_PER_GENERATION);
	predator.epoch(STEPS_PER_GENERATION);
}

void env_food::pre_tick() {
	herbivores.pre_tick();
	predator.pre_tick();
}

void env_food::tick() {
	herbivores.tick();
	predator.tick();
	for(auto &env_obj : environmental_objects) {
		env_obj->tick();
	}
}

void env_food::draw() {
	// Draw environmental objects
	for(const auto &env_obj : environmental_objects){
		env_obj->draw(conf.projection);
	}
	//draw agents
	predator.draw(conf.projection);
	herbivores.draw(conf.projection);
}

}
