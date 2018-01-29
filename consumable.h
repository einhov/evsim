#ifndef CONSUMABLE_H
#define CONSUMABLE_H

#include <array>

#include <Box2D/Box2D.h>

#include "fixture_type.h"
#include "entity.h"

namespace evsim {

struct msg_contact {
	b2Fixture *fixture_native;
	b2Fixture *fixture_foreign;
};

struct msg_consume {
	entity *consumer;
};

struct msg_consumed {
};

class consumable : public entity {
	public:
		void message(const std::any &msg) override;

		b2Body *body;

		void init_body(b2World &world);
		void update();
		void draw(const glm::mat4 &projection) const;
};

};
#endif
