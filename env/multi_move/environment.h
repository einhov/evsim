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
		const int STEPS_PER_GENERATION = 5;
		const int TICKS_PER_STEP = 60 * 15;
};

}
}

#endif
