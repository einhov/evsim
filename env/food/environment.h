#ifndef ENV_FOOD_H
#define ENV_FOOD_H

#include "../../environment_base.h"

#include "herbivore_neat.h"

namespace evsim {
namespace food {

class environment : public environment_base {
	public:
		environment();
		~environment() {}
		void init() override;
		void step() override;
		void epoch() override;
		void pre_tick() override;
		void tick() override;
		void draw() override;
		int steps_per_generation() override { return STEPS_PER_GENERATION; }
		int ticks_per_step() override { return TICKS_PER_STEP; }
		herbivore_neat herbivores;
		static constexpr int STEPS_PER_GENERATION = 5;
		static constexpr int TICKS_PER_STEP = 60 * 15;
};

}
}

#endif
