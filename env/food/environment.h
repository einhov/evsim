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
		void init(lua_conf &conf) override;
		void step() override;
		void pre_step() override;
		void epoch() override;
		void pre_tick() override;
		void tick() override;
		void draw() override;
		unsigned int steps_per_generation() override { return params.steps_per_generation; }
		unsigned int ticks_per_step() override { return params.ticks_per_step; }
		herbivore_neat herbivores;

		struct {
			unsigned int steps_per_generation;
			unsigned int ticks_per_step;
		} params;
};

}
}

#endif
