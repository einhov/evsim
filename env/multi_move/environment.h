#ifndef ENV_MULTI_MOVE_H
#define ENV_MULTI_MOVE_H

#include "../../environment_base.h"

#include "herbivore_neat.h"
#include "predator_neat.h"

namespace evsim {
namespace multi_move {

class environment : environment_base {
	public:
		environment();
		void init();
		void step();
		void epoch();
		void pre_tick();
		void tick();
		void draw();
		herbivore_neat herbivores;
		predator_neat predator;
		size_t step_count;
		const unsigned int STEPS_PER_GENERATION = 25;
		const unsigned int TICKS_PER_STEP = 60 * 15;
};

}
}

#endif
