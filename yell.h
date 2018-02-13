#ifndef YELL_H
#define YELL_H

#include <array>

#include <Box2D/Box2D.h>

#include "entity.h"
#include "species_neat.h"

namespace evsim {

class yell : public environmental_entity {
	public:
		int ticks_to_live = 60;
		void tick() override;
		void schedule_stop_yell();
		void message(const std::any &msg) override {}
		b2Body* body;
		b2World* world;
		species_neat::agent* holler;

		void init_body(b2World &world, species_neat::agent *agent);
		void draw(const glm::mat4 &projection) const override;
		~yell() override;
};

};
#endif
