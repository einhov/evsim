#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

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

environment::environment() : herbivores(*conf.world), predator(*conf.world) {}

void environment::init() {

	herbivores.initialise(build_config::herbivore_count, static_cast<int>(glfwGetTime()));
//	predator.initialise(build_config::predator_count, static_cast<int>(glfwGetTime()+1));

/*
	float pos[8] = {
		0.0f, 100.0f,
		0.0f, -100.0f,
		-100.0f* (4.0f / 3.0f), 0.0f,
		100.0f* (4.0f / 3.0f), 0.0f
		};
	float scale[4] = {
		100.0f* (4.0f / 3.0f), 1.0f,
		1.0f* (4.0f / 3.0f), 100.0f
		};
	for(int i = 0; i < 8; i+=2) {
		auto wall_instance = std::make_unique<wall>();
		b2Vec2 p (pos[i], pos[i+1]);
		b2Vec2 s (scale[((i)/4)*2], scale[((i)/4)*2+1]);
		wall_instance->init_body(
			(*conf.world),
			p,
			s
			);
		environmental_objects.emplace_back(std::move(wall_instance));
	}
*/
}


void environment::step() {
	herbivores.step();
//	predator.step();
}

void environment::epoch() {
	herbivores.epoch(STEPS_PER_GENERATION);
//	predator.epoch(STEPS_PER_GENERATION);
}

void environment::pre_tick() {
	herbivores.pre_tick();
//	predator.pre_tick();
}

void environment::tick() {
	herbivores.tick();
//	predator.tick();
	for(auto &env_obj : environmental_objects) {
		env_obj->tick();
	}
}

void environment::draw() {
	// Draw environmental objects
	for(const auto &env_obj : environmental_objects){
		env_obj->draw(conf.projection);
	}
	//draw agents
	herbivores.draw(conf.projection);
//	predator.draw(conf.projection);
}

}
}
