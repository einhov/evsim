#ifndef EVSIM_H
#define EVSIM_H
#include <vector>
#include "yell.h"

namespace evsim {

struct configuration {
	bool draw_sensors_herbivore;
	bool draw_sensors_predator;
};

extern std::vector<std::unique_ptr<environmental_entity>> environmental_objects;
extern std::vector<environmental_entity*> to_be_destroyed;
extern configuration conf;

}

#endif
