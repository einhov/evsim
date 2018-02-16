#ifndef ENVIRONMENT1_H
#define ENVIRONMENT1_H

#include <glm/glm.hpp>

namespace evsim {

class environment_base {
	public:
		virtual void init() = 0;
		virtual void draw() = 0;
		virtual void step() = 0;
		virtual void epoch() = 0;
		virtual void pre_tick() = 0;
		virtual void tick() = 0;
		virtual ~environment_base() {}
};

}

#endif
