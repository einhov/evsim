#ifndef EVSIM_H
#define EVSIM_H

#include <mutex>

namespace evsim {

struct simulation_state {
	int generation;
	int step;
	int tick;

	bool quit;
	bool pause;
	bool draw;
	std::mutex mutex;
};

extern simulation_state state;

}

#endif
