#ifndef WALL_H
#define WALL_H

#include <optional>

#include <Box2D/Box2D.h>

#include "entity.h"

namespace evsim {

enum class wall_type {
	standard,
	right,
	goal,
	button
};

class wall : public environmental_entity {
	public:
		~wall() override;
		void tick() override;
		void message(const std::any &msg) override {}
		void draw(const glm::mat4 &projection) const override;
		void set_active(bool active);
		void init_body(b2World &world, b2Vec2& position, b2Vec2& scale, wall_type right_wall);

		wall_type type;
		b2Body* body;
		b2World* world;
		b2Vec2 scale;

		std::optional<unsigned int> door_id;
		bool active = true;
};

}

#endif
