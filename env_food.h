#ifndef ENV_FOOD_H
#define ENV_FOOD_H

#include "environment.h"
#include "species_neat.h"
#include "predator_neat.h"

namespace evsim {

class env_food : environment {
	public:
		env_food();
		void init();
		void step();
		void epoch();
		void pre_tick();
		void tick();
		void draw();
		species_neat herbivores;
		predator_neat predator;
		const int STEPS_PER_GENERATION = 5;
		const int TICKS_PER_STEP = 60 * 15;
};

}

#endif
