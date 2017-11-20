#ifndef DATATYPES_H
#define DATATYPES_H

#include <vector>
#include "Box2D/Box2D.h"
#include <string>

struct Force_increment {
	float angular_force;
	float linear_force;
};

struct Agent_state {
	std::vector<float> sensor_poison;
	std::vector<float> sensor_food;
	b2Vec2 Linear_velocity;
	float Angular_velocity;
};

struct GA_node {
	std::string name;
	std::vector<std::string> values;
};

struct GA_data_in {
	std::vector<GA_node> input_nodes;
	std::vector<GA_node> output_nodes;
};

struct Edge {
	GA_node input_node;
	GA_node output_node;
	int input_value_index;
	int output_value_index;
};



#endif // DATATYPES_H
