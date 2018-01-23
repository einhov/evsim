#ifndef BODY_H
#define BODY_H

namespace evsim {

enum class fixture_type {
	sensor_left,
	sensor_right,
	torso
};

b2Body *build_body(b2World &world);

extern const std::array<b2Vec2, 3> sensor_left;
extern const std::array<b2Vec2, 3> sensor_right;

};

#endif
