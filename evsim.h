#ifndef EVSIM_H
#define EVSIM_H

#include <optional>
#include <vector>
#include <glm/glm.hpp>

#include <QCoreApplication>
#include "entity.h"

class gui;
class b2World;

namespace evsim {

struct simulation_state {
	int generation;
	unsigned int step;
	unsigned int tick;

	bool quit;
	bool pause;
	bool draw;
	bool skip;
	bool previous_step;
	std::optional<unsigned int> next_step;

	bool draw_yell;
	bool draw_wall;

	float simulation_timestep;
	glm::mat4 projection;
	b2World *world;
};

extern std::vector<std::unique_ptr<environmental_entity>> environmental_objects;
extern std::vector<environmental_entity*> to_be_destroyed;
extern simulation_state state;
extern gui *main_gui;

}

#endif
