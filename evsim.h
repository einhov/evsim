#ifndef EVSIM_H
#define EVSIM_H

#include <mutex>

#include <vector>
#include <glm/glm.hpp>

#include <QCoreApplication>

#include "ui/gui.h"
#include "yell.h"

namespace evsim {

struct simulation_state {
	int generation;
	unsigned int step;
	unsigned int tick;

	bool quit;
	bool pause;
	bool draw;
	bool fast_forward;
	bool previous_step;

	bool draw_yell;
	bool draw_wall;

	float simulation_timestep;
	glm::mat4 projection;
	b2World *world;

	std::mutex mutex;
};

extern std::vector<std::unique_ptr<environmental_entity>> environmental_objects;
extern std::vector<environmental_entity*> to_be_destroyed;
extern simulation_state state;
extern gui *main_gui;

}

#endif
