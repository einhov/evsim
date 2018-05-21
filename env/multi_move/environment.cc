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
#include "predator_neat.h"

namespace evsim {
namespace multi_move {

using training_model_type = evsim::species::training_model_type;

environment::environment() : herbivores(*state.world, *this), predator(*state.world, *this) {}

void environment::init(lua_conf &conf) {
	params.ticks_per_step = conf.get_integer_default("ticks_per_step", 60 * 15);
	params.steps_per_generation = conf.get_integer_default("steps_per_generation", 50);

	conf.enter_table_or_empty("herbivores");
	herbivores.initialise(conf, static_cast<int>(glfwGetTime()));
	conf.leave_table();
	conf.enter_table_or_empty("predators");
	predator.initialise(conf, static_cast<int>(glfwGetTime()+1));
	conf.leave_table();

	if(herbivores.is_sharedish(herbivores.training_model()) && predator.is_sharedish(predator.training_model())) {
		throw std::runtime_error("Both herbivore and predator have shared fitness, this is not allowed!");
	}

	const bool herbivore_shared = herbivores.training_model() == training_model_type::shared;
	const bool predator_shared = predator.training_model() == training_model_type::shared;
	if(
		(herbivore_shared && params.steps_per_generation != herbivores.population_size()) ||
		(predator_shared && params.steps_per_generation != predator.population_size())
	) {
		throw std::runtime_error(
			"The steps_per_generation must be equal to the population_size of the agent that is trained with shared_fitness"
		);
	}

	QApplication::postEvent(main_gui, new gui::add_species_event(&herbivores));
	QApplication::postEvent(main_gui, new gui::add_species_event(&predator));

	const bool no_training_mode =
		!herbivores.train() && !predator.train() &&
		(herbivores.training_model() != training_model_type::shared_eval) &&
		(predator.training_model() != training_model_type::shared_eval)
	;
	if(no_training_mode)
		QApplication::postEvent(main_gui, new gui::no_training_mode_event());

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
				s,
				wall_type::standard
				);
			environmental_objects.emplace_back(std::move(wall_instance));
		}
		y = 100;
		//Creating vertical walls
		for(int j = -100; j < 100; j+=2) {
			auto wall_instance = std::make_unique<wall>();
			b2Vec2 p (x, j);
			b2Vec2 s (2, 2);
			if(i == 0) {
				wall_instance->init_body(
					(*state.world), p, s, wall_type::standard
				);
			}
			else {
				wall_instance->init_body(
					(*state.world), p, s, wall_type::right
				);
			}

			environmental_objects.emplace_back(std::move(wall_instance));
		}
		x = 134;
	}

	//Create goal
	for(int y = -100; y < 100; y++) {
		auto wall_instance = std::make_unique<wall>();
		b2Vec2 p (101, y);
		b2Vec2 s (1, 1);
		wall_instance->init_body(
			(*state.world), p, s, wall_type::goal
		);
		environmental_objects.emplace_back(std::move(wall_instance));
	}
}

void environment::pre_step() {
	herbivores.pre_step();
	predator.pre_step();

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
		case training_model_type::shared_eval:
			herbivores.step_shared_eval(state.step);
	}

	switch(predator.training_model()) {
		case training_model_type::normal:
			predator.step_normal();
			break;
		case training_model_type::shared:
			predator.step_shared(state.step);
			break;
		case training_model_type::shared_eval:
			predator.step_shared_eval(state.step);
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
		case training_model_type::shared_eval:
			herbivores.epoch_shared_eval(state.generation);
	}

	switch(predator.training_model()) {
		case training_model_type::normal:
			predator.epoch_normal(state.generation, steps_per_generation());
			break;
		case training_model_type::shared:
			predator.epoch_shared(state.generation);
			break;
		case training_model_type::shared_eval:
			predator.epoch_shared_eval(state.generation);
	}

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
	const auto projection = state.camera.projection();

	// Draw environmental objects
	for(const auto &env_obj : boost::adaptors::reverse(environmental_objects)){
		env_obj->draw(projection);
	}
	//draw agents
	predator.draw(projection);
	herbivores.draw(projection);
}

}
}
