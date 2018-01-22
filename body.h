#ifndef BODY_H
#define BODY_H

namespace evsim {

enum class fixture_type {
	sensor_left,
	sensor_right,
	torso
};

b2Body *build_body(b2World &world);

};

#endif
