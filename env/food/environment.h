#ifndef ENV_FOOD_H
#define ENV_FOOD_H

#include "../../environment_base.h"

namespace evsim {
namespace food {

class environment : environment_base {
	public:
		environment();
		void init();
		void step();
		void epoch();
		void pre_tick();
		void tick();
		void draw();
		const int STEPS_PER_GENERATION = 5;
		const int TICKS_PER_STEP = 60 * 15;
};

}
}

#endif
