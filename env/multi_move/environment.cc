#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <QApplication>

#include "../../environment_base.h"
#include "../../species.h"
#include "../../evsim.h"
#include "../../config.h"
#include "../../consumable.h"
#include "../../wall.h"

#include "environment.h"
#include "herbivore_neat.h"
#include "predator_neat.h"

namespace evsim {
namespace multi_move {

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

	int y = -100;
	int x = -134;
	for(int i = 0; i < 2; i++) {
		//Creating horisontal walls
		for(int j = -134; j < 134; j+=2) {
			auto wall_instance = std::make_unique<wall>();
			b2Vec2 p (j, y);
			b2Vec2 s (2, 2);
			wall_instance->init_body(
				(*state.world),
				p,
				s
				);
			environmental_objects.emplace_back(std::move(wall_instance));
		}
		y = 100;
		//Creating vertical walls
		for(int j = -100; j < 100; j+=2) {
			auto wall_instance = std::make_unique<wall>();
			b2Vec2 p (x, j);
			b2Vec2 s (2, 2);
			wall_instance->init_body(
				(*state.world),
				p,
				s
				);
			environmental_objects.emplace_back(std::move(wall_instance));
		}
		x = 134;
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
