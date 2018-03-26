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

using training_model_type = evsim::species::training_model_type;

environment::environment() : herbivores(*state.world), predator(*state.world) {}

void environment::init(lua_conf &conf) {
	const auto food_count = conf.get_integer_default("food_count", 150);
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

	if(!herbivores.train() && !predator.train())
		QApplication::postEvent(main_gui, new gui::no_training_mode_event());

	for(int i = 0; i < food_count; i++) {
		auto consumable_instance = std::make_unique<consumable>();
		consumable_instance->init_body((*state.world));
		environmental_objects.emplace_back(std::move(consumable_instance));
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
	}

	switch(predator.training_model()) {
		case training_model_type::normal:
			predator.step_normal();
			break;
		case training_model_type::shared:
			predator.step_shared(state.step);
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

	switch(predator.training_model()) {
		case training_model_type::normal:
			predator.epoch_normal(state.generation, steps_per_generation());
			break;
		case training_model_type::shared:
			predator.epoch_shared(state.generation);
			break;
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
