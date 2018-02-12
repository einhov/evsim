#ifndef ENTITY_H
#define ENTITY_H

#include <any>
#include <Box2D/Box2D.h>

namespace evsim {

class entity {
	public:
		virtual void message(const std::any &msg) = 0;
};

class environmental_entity : public entity{
	public:
		virtual void tick() = 0;
		virtual void draw(const glm::mat4 &projection) const = 0;
};

struct msg_contact {
	b2Fixture *fixture_native;
	b2Fixture *fixture_foreign;
};

};

#endif
