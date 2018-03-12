#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <QApplication>

#include "../../environment_base.h"
#include "../../evsim.h"
#include "../../config.h"
#include "../../consumable.h"

#include "environment.h"
#include "herbivore_neat.h"

namespace evsim {
namespace food {

environment::environment() : herbivores(*state.world) {}

void environment::init() {
	herbivores.initialise(build_config::herbivore_count, static_cast<int>(glfwGetTime()));

	if((herbivores.shared_fitness && STEPS_PER_GENERATION != build_config::herbivore_count)) {
		throw std::runtime_error(
			"The STEPS_PER_GENERATION must be equal to the population_size of the agent that is trained with shared_fitness!"
		);
	}

	QApplication::postEvent(main_gui, new gui::add_species_event(&herbivores));

	for(size_t i = 0; i < build_config::food_count; i++) {
		auto consumable_instance = std::make_unique<consumable>();
		consumable_instance->init_body((*state.world));
		environmental_objects.emplace_back(std::move(consumable_instance));
	}
	step_count = 0;
}

void environment::step() {
	if(herbivores.shared_fitness) {
		herbivores.step_shared_fitness(step_count);
		step_count++;
	}
	else {
		herbivores.step();
	}
}

void environment::epoch() {
	if(herbivores.shared_fitness) {
		step_count = 0;
		herbivores.epoch_shared_fitness();
	}
	else {
		herbivores.epoch(STEPS_PER_GENERATION);
	}
}

void environment::pre_tick() {
	herbivores.pre_tick();
}

void environment::tick() {
	herbivores.tick();
	for(auto &env_obj : environmental_objects) {
		env_obj->tick();
	}
}

void environment::draw() {
	// Draw environmental objects
	for(const auto &env_obj : boost::adaptors::reverse(environmental_objects)){
		env_obj->draw(state.projection);
	}
	//draw agents
	herbivores.draw(state.projection);
}

}
}
