#ifndef DATATYPES_H
#define DATATYPES_H

#include <vector>
#include "Box2D/Box2D.h"

struct force_increment {
	float angular_force;
	float linear_force;
};

struct agent_state {
	std::vector<float> sensor_poison;
	std::vector<float> sensor_food;
	b2Vec2 Linear_velocity;
	float Angular_velocity;
};


#endif // DATATYPES_H
