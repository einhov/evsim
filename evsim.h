#ifndef EVSIM_H
#define EVSIM_H

#include <vector>
#include <glm/glm.hpp>

#include "yell.h"

namespace evsim {

struct configuration {
	bool draw_sensors_herbivore;
	bool draw_sensors_predator;
	bool draw_yell;
	bool draw_wall;
	bool draw;
	bool pause;
	int generation;
	int tick;
	int step;
	float simulation_timestep;
	glm::mat4 projection;
	b2World *world;
};

extern std::vector<std::unique_ptr<environmental_entity>> environmental_objects;
extern std::vector<environmental_entity*> to_be_destroyed;
extern configuration conf;

}

#endif
