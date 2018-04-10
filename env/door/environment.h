#ifndef ENV_DOOR_H
#define ENV_DOOR_H

#include "../../environment_base.h"

#include "herbivore_neat.h"

namespace evsim {
namespace door {

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
		herbivore_neat herbivores;
		void set_button_active(int id);
		std::vector<int> button_status;

		struct {
			unsigned int steps_per_generation;
			unsigned int ticks_per_step;
		} params;

	private:
		void reset_status();
		void setDoorsActive();
};

}
}

#endif
