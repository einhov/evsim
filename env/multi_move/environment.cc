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
		env_obj->draw(state.projection);
	}
	//draw agents
	predator.draw(state.projection);
	herbivores.draw(state.projection);
}

}
}
