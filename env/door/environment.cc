#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <QApplication>
#include <boost/range/adaptor/reversed.hpp>

#include "../../environment_base.h"
#include "../../species.h"
#include "../../evsim.h"
#include "../../config.h"
#include "../../consumable.h"
#include "../../wall.h"
#include "../../ui/gui.h"

#include "environment.h"
#include "herbivore_neat.h"

namespace evsim {
namespace door {

using training_model_type = evsim::species::training_model_type;

environment::environment() : herbivores(*state.world, *this) {}

void environment::init(lua_conf &conf) {
	params.ticks_per_step = conf.get_integer_default("ticks_per_step", 60 * 15);
	params.steps_per_generation = conf.get_integer_default("steps_per_generation", 50);

	conf.enter_table_or_empty("herbivores");
	herbivores.initialise(conf, static_cast<int>(glfwGetTime()));
	conf.leave_table();

	if(
		(herbivores.training_model() == herbivore_neat::training_model_type::shared &&
		 params.steps_per_generation != herbivores.population_size())) {
		throw std::runtime_error(
			"The steps_per_generation must be equal to the population_size of the agent that is trained with shared_fitness"
		);
	}

	QApplication::postEvent(main_gui, new gui::add_species_event(&herbivores));

	if(!herbivores.train())
		QApplication::postEvent(main_gui, new gui::no_training_mode_event());

	// Create doors
	int y = -50;
	int x = -20;
	for(int i = 0; i < 2; i++) {
		// Creating horisontal doors
		for(int j = -20; j < 20; j++) {
			auto door_instance = std::make_unique<wall>();
			b2Vec2 p (j, y);
			b2Vec2 s (1, 1);
			door_instance->init_body(
				(*state.world),
				p,
				s,
				wall_type::standard
			);
			door_instance->door_id = 0;
			doors.emplace_back(std::ref(*door_instance));
			environmental_objects.emplace_back(std::move(door_instance));
		}
		y = 50;
		// Creating vertical doors
		for(int j = -50; j < 50; j+=1) {
			auto door_instance = std::make_unique<wall>();
			b2Vec2 p (x, j);
			b2Vec2 s (1, 1);
			door_instance->init_body(
				(*state.world),
				p,
				s,
				wall_type::standard
			);
			door_instance->door_id = 0;
			doors.emplace_back(std::ref(*door_instance));
			environmental_objects.emplace_back(std::move(door_instance));
		}
		x = 20;
	}

	// Create goal
	for(int j = -40; j < 40; j++) {
		auto wall_instance = std::make_unique<wall>();
		b2Vec2 p (0, j);
		b2Vec2 s (1, 1);
		wall_instance->init_body(
			(*state.world), p, s, wall_type::goal
		);
		environmental_objects.emplace_back(std::move(wall_instance));
	}

	// Create button
	auto wall_instance = std::make_unique<wall>();
	b2Vec2 p (50, 0);
	b2Vec2 s (10, 10);
	wall_instance->init_body(
		(*state.world), p, s, wall_type::button
	);
	environmental_objects.emplace_back(std::move(wall_instance));
	button_status.emplace_back(0);
}

void environment::pre_step() {
	herbivores.pre_step();

	for(auto &env_obj : environmental_objects)
		env_obj->pre_step();
}

void environment::step() {
	switch(herbivores.training_model()) {
		case training_model_type::normal:
			herbivores.step_normal();
			break;
		case training_model_type::shared:
			herbivores.step_shared(state.step);
			break;
	}
}

void environment::epoch() {
	switch(herbivores.training_model()) {
		case training_model_type::normal:
			herbivores.epoch_normal(state.generation, steps_per_generation());
			break;
		case training_model_type::shared:
			herbivores.epoch_shared(state.generation);
			break;
	}
}

void environment::pre_tick() {
	set_door_state();
	reset_status();
	herbivores.pre_tick();
}

void environment::tick() {
	herbivores.tick();
	for(auto &env_obj : environmental_objects) {
		env_obj->tick();
	}
}

void environment::draw() {
	const auto projection = state.camera.projection();

	for(const auto &env_obj : boost::adaptors::reverse(environmental_objects)){
		env_obj->draw(projection);
	}

	herbivores.draw(projection);
}

void environment::reset_status() {
	for(auto &status : button_status) {
		status = 0;
	}
}

void environment::set_button_active(int id) {
	button_status[id]++;
}

void environment::set_door_state() {
	for(size_t i = 0; i < button_status.size(); i++) {
		for (wall &door : doors) {
			if(door.door_id == i) {
				door.set_active(button_status[i] <= 0);
			}
		}
	}
}

}
}
