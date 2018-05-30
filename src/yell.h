#ifndef YELL_H
#define YELL_H

#include <array>

#include <Box2D/Box2D.h>

#include "entity.h"

namespace evsim {

class yell : public environmental_entity {
	public:
		int ticks_to_live = 60;
		void tick() override;
		void pre_step() override;
		void schedule_stop_yell();
		void message(const std::any &msg) override {}
		b2Body* body;
		b2World* world;
		entity *hollerer;

		void init_body(b2World &world, entity *hollerer, const b2Vec2& position);
		void draw(const glm::mat4 &projection) const override;
		~yell() override;
};

}

#endif
