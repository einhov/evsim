#ifndef EVSIM_H
#define EVSIM_H

namespace evsim {

struct configuration {
	bool draw_sensors_herbivore;
	bool draw_sensors_predator;
};

extern configuration conf;

}

#endif
