#ifndef ENV_DOOR_H
#define ENV_DOOR_H

#include "../../environment_base.h"

#include "herbivore_neat.h"

namespace evsim {
class wall;
}

namespace evsim::door {

class environment : public environment_base {
	public:
		environment();
		~environment() {}
		void init(lua_conf &conf) override;
		void pre_step() override;
		void step() override;
		void epoch() override;
		void pre_tick() override;
		void tick() override;
		void draw() override;
		unsigned int steps_per_generation() override { return params.steps_per_generation; }
		unsigned int ticks_per_step() override { return params.ticks_per_step; }
		void set_button_active(int id);

		struct {
			unsigned int steps_per_generation;
			unsigned int ticks_per_step;
		} params;

	private:
		void reset_status();
		void set_door_state();
		herbivore_neat herbivores;
		std::vector<int> button_status;
		std::vector<std::reference_wrapper<wall>> doors;
};

}

#endif
