#ifndef ENVIRONMENT_BASE_H
#define ENVIRONMENT_BASE_H

namespace evsim {

class environment_base {
	public:
		virtual ~environment_base() {}
		virtual void init() = 0;
		virtual void draw() = 0;
		virtual void step() = 0;
		virtual void epoch() = 0;
		virtual void pre_tick() = 0;
		virtual void tick() = 0;

		virtual int steps_per_generation() = 0;
		virtual int ticks_per_step() = 0;
};

}

#endif
