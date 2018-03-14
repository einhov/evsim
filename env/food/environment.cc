#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <QApplication>
#include <boost/range/adaptor/reversed.hpp>

#include "../../environment_base.h"
#include "../../evsim.h"
#include "../../config.h"
#include "../../consumable.h"

#include "environment.h"
#include "herbivore_neat.h"

namespace evsim {
namespace food {

using training_model_type = evsim::species::training_model_type;

environment::environment() : herbivores(*state.world) {}

void environment::init(lua_conf &conf) {
	const auto food_count = conf.get_integer_default("food_count", 150);
	params.ticks_per_step = conf.get_integer_default("ticks_per_step", 60 * 15);
	params.steps_per_generation = conf.get_integer_default("steps_per_generation", 50);

	conf.enter_table_or_empty("herbivores");
	herbivores.initialise(conf, static_cast<int>(glfwGetTime()));
	conf.leave_table();

	if((herbivores.training_model() == training_model_type::shared && params.steps_per_generation != herbivores.population_size())) {
		throw std::runtime_error(
			"The steps_per_generation must be equal to the population_size of the agent that is trained with shared_fitness"
		);
	}

	QApplication::postEvent(main_gui, new gui::add_species_event(&herbivores));

	for(int i = 0; i < food_count; i++) {
		auto consumable_instance = std::make_unique<consumable>();
		consumable_instance->init_body((*state.world));
		environmental_objects.emplace_back(std::move(consumable_instance));
	}
	step_count = 0;
}

void environment::step() {
	for(auto &env_obj : environmental_objects) {
		env_obj->step();
	}

	switch(herbivores.training_model()) {
		case training_model_type::normal_none: [[fallthrough]]
		case training_model_type::normal:
			herbivores.step();
			return;
		case training_model_type::shared:
			herbivores.step_shared_fitness(step_count++);
			return;
		case training_model_type::shared_none:
			throw std::runtime_error("shared_none training model unimplemented for environment");
	}
}

void environment::epoch() {
	switch(herbivores.training_model()) {
		case training_model_type::normal:
			herbivores.epoch(steps_per_generation());
			return;
		case training_model_type::shared:
			step_count = 0;
			herbivores.epoch_shared_fitness();
			return;
		case training_model_type::normal_none:
			herbivores.epoch_normal_none(state.generation, steps_per_generation());
			return;
		case training_model_type::shared_none:
			throw std::runtime_error("shared_none training model unimplemented for environment");
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
