#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <QApplication>
#include <boost/range/adaptor/reversed.hpp>

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

environment::environment() : herbivores(*state.world), predator(*state.world) {}

void environment::init() {
	herbivores.initialise(build_config::herbivore_count, static_cast<int>(glfwGetTime()));
	predator.initialise(build_config::predator_count, static_cast<int>(glfwGetTime()+1));

	if(herbivores.shared_fitness && predator.shared_fitness) {
		throw std::runtime_error("Both herbivore and predator have shared fitness, this is not allowed!");
	}
	if((herbivores.shared_fitness && STEPS_PER_GENERATION != build_config::herbivore_count) ||
	   (predator.shared_fitness && STEPS_PER_GENERATION != build_config::predator_count)) {
		throw std::runtime_error(
			"The STEPS_PER_GENERATION must be equal to the population_size of the agent that is trained with shared_fitness!"
		);
	}

	QApplication::postEvent(main_gui, new gui::add_species_event(&herbivores));
	QApplication::postEvent(main_gui, new gui::add_species_event(&predator));

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
	}
	else {
		herbivores.step();
	}
	if(predator.shared_fitness) {
		predator.step_shared_fitness(step_count);
	}
	else {
		predator.step();
	}
	step_count++;
}

void environment::epoch() {
	if(herbivores.shared_fitness) {
		herbivores.epoch_shared_fitness();
	}
	else {
		herbivores.epoch(STEPS_PER_GENERATION);
	}
	if(predator.shared_fitness) {
		predator.epoch_shared_fitness();
	}
	else {
		predator.epoch(STEPS_PER_GENERATION);
	}
	step_count = 0;
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
	for(const auto &env_obj : boost::adaptors::reverse(environmental_objects)){
		env_obj->draw(state.projection);
	}
	//draw agents
	predator.draw(state.projection);
	herbivores.draw(state.projection);
}

}
}
