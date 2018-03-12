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

void environment::init(lua_conf &conf) {
	const auto food_count = conf.get_integer_default("food_count", 150);
	conf.enter_table_or_empty("herbivores");
	herbivores.initialise(conf, static_cast<int>(glfwGetTime()));
	conf.leave_table();

	QApplication::postEvent(main_gui, new gui::add_species_event(&herbivores));

	for(int i = 0; i < food_count; i++) {
		auto consumable_instance = std::make_unique<consumable>();
		consumable_instance->init_body((*state.world));
		environmental_objects.emplace_back(std::move(consumable_instance));
	}
}

void environment::step() {
	herbivores.step();
}

void environment::epoch() {
	herbivores.epoch(STEPS_PER_GENERATION);
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
	for(const auto &env_obj : environmental_objects){
		env_obj->draw(state.projection);
	}
	//draw agents
	herbivores.draw(state.projection);
}

}
}
