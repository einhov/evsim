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
	int step;
	int tick;

	bool quit;
	bool pause;
	bool draw;

	bool draw_yell;

	bool draw_sensors_herbivore;
	bool draw_sensors_predator;

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
