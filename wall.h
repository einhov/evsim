#ifndef WALL_H
#define WALL_H

#include <array>

#include <Box2D/Box2D.h>

#include "entity.h"

namespace evsim {

class wall : public environmental_entity {
	public:
		void tick() override;
		void message(const std::any &msg) override {}
		void draw(const glm::mat4 &projection) const override;
		~wall() override;

		void init_body(b2World &world, b2Vec2& position, b2Vec2& scale);

		b2Body* body;
		b2World* world;
		b2Vec2 scale;

};

};
#endif
