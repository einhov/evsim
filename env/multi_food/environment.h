#ifndef ENV_MULTI_FOOD_H
#define ENV_MULTI_FOOD_H

#include "../../environment_base.h"

#include "herbivore_neat.h"
#include "predator_neat.h"

namespace evsim {
namespace multi_food {

class environment : public environment_base {
	public:
		environment();
		~environment() {}
		void init(lua_conf &conf) override;
		void step() override;
		void epoch() override;
		void pre_tick() override;
		void tick() override;
		void draw() override;
		int steps_per_generation() override { return STEPS_PER_GENERATION; }
		int ticks_per_step() override { return TICKS_PER_STEP; }
		herbivore_neat herbivores;
		predator_neat predator;

		size_t step_count;
		const unsigned int STEPS_PER_GENERATION = 25;
		const unsigned int TICKS_PER_STEP = 60 * 15;

};

}
}

#endif
