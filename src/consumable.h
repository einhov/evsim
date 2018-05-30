#ifndef CONSUMABLE_H
#define CONSUMABLE_H

#include <array>

#include <Box2D/Box2D.h>

#include "entity.h"

namespace evsim {

struct msg_consume {
	entity *consumer;
};

struct msg_consumed {
};

class consumable : public environmental_entity {
	public:
		void message(const std::any &msg) override;
		void pre_step() override;

		b2Body *body;

		void init_body(b2World &world);
		void draw(const glm::mat4 &projection) const override;
};

}

#endif
