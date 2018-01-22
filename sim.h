#ifndef AGENT_H
#define AGENT_H

#include <array>

#include <Genome.h>
#include <NeuralNetwork.h>
#include <Box2D/Box2D.h>

namespace evsim {

struct food {
	b2Body *body;

	void init_body(b2World &world);
	void update();
};

extern const std::array<b2Vec2, 3> sensor_left;
extern const std::array<b2Vec2, 3> sensor_right;

};
#endif
