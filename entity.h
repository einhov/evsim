#ifndef ENTITY_H
#define ENTITY_H

#include <any>
#include <Box2D/Box2D.h>
#include <glm/glm.hpp>

namespace evsim {

class entity {
	public:
		virtual void message(const std::any &msg) = 0;
};

class environmental_entity : public entity {
	public:
		virtual void tick() {}
		virtual void pre_step() {}
		virtual void draw(const glm::mat4 &projection) const = 0;
		virtual void set_active(bool active) {}
		virtual ~environmental_entity() {}
		int id = -1;
		bool door = false;
		bool active = true;
};

struct msg_contact {
	b2Fixture *fixture_native;
	b2Fixture *fixture_foreign;
};

}

#endif
