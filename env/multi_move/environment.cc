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

#include "environment.h"
#include "herbivore_neat.h"
#include "predator_neat.h"

namespace evsim {
namespace multi_move {

using training_model_type = evsim::species::training_model_type;

environment::environment() : herbivores(*state.world), predator(*state.world) {}

void environment::init(lua_conf &conf) {
	params.ticks_per_step = conf.get_integer_default("ticks_per_step", 60 * 15);
	params.steps_per_generation = conf.get_integer_default("steps_per_generation", 50);

	conf.enter_table_or_empty("herbivores");
	herbivores.initialise(conf, static_cast<int>(glfwGetTime()));
	conf.leave_table();
	conf.enter_table_or_empty("predators");
	predator.initialise(conf, static_cast<int>(glfwGetTime()+1));
	conf.leave_table();

	if(
		herbivores.training_model() == herbivore_neat::training_model_type::shared &&
		predator.training_model() == predator_neat::training_model_type::shared
	) {
		throw std::runtime_error("Both herbivore and predator have shared fitness, this is not allowed!");
	}
	if(
		(herbivores.training_model() == herbivore_neat::training_model_type::shared &&
		 params.steps_per_generation != herbivores.population_size()) ||
		(predator.training_model() == predator_neat::training_model_type::shared &&
		 params.steps_per_generation != predator.population_size())
	) {
		throw std::runtime_error(
			"The steps_per_generation must be equal to the population_size of the agent that is trained with shared_fitness"
		);
	}

	QApplication::postEvent(main_gui, new gui::add_species_event(&herbivores));
	QApplication::postEvent(main_gui, new gui::add_species_event(&predator));

	static constexpr auto is_training = [](const auto &species) {
		const auto training_model = species.training_model();
		return training_model == training_model_type::normal || training_model == training_model_type::shared;
	};

	if(!is_training(herbivores) && !is_training(predator))
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
				false
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
					(*state.world),
					p,
					s,
					false
					);
			}
			else {
				wall_instance->init_body(
					(*state.world),
					p,
					s,
					true
					);
			}

			environmental_objects.emplace_back(std::move(wall_instance));
		}
		x = 134;
	}
}


void environment::step() {
	for(auto &env_obj : environmental_objects) {
		env_obj->step();
	}

	switch(herbivores.training_model()) {
		case training_model_type::normal_none: [[fallthrough]]
		case training_model_type::normal:
			herbivores.step();
			break;
		case training_model_type::shared_none: [[fallthrough]]
		case training_model_type::shared:
			herbivores.step_shared_fitness(state.step);
			break;
	}

	switch(predator.training_model()) {
		case training_model_type::normal_none: [[fallthrough]]
		case training_model_type::normal:
			predator.step();
			break;
		case training_model_type::shared_none: [[fallthrough]]
		case training_model_type::shared:
			predator.step_shared_fitness(state.step);
			break;
	}
}

void environment::epoch() {
	switch(herbivores.training_model()) {
		case training_model_type::normal:
			herbivores.epoch(steps_per_generation());
			break;
		case training_model_type::shared:
			herbivores.epoch_shared_fitness(state.generation, true);
			break;
		case training_model_type::normal_none:
			herbivores.epoch_normal_none(state.generation, steps_per_generation());
			break;
		case training_model_type::shared_none:
			herbivores.epoch_shared_fitness(state.generation, false);
			break;
	}

	switch(predator.training_model()) {
		case training_model_type::normal:
			predator.epoch(steps_per_generation());
			break;
		case training_model_type::shared:
			predator.epoch_shared_fitness(state.generation, true);
			break;
		case training_model_type::normal_none:
			predator.epoch_normal_none(state.generation, steps_per_generation());
			break;
		case training_model_type::shared_none:
			predator.epoch_shared_fitness(state.generation, false);
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
