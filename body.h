#ifndef BODY_H
#define BODY_H

namespace evsim {

enum class fixture_type {
	sensor,
	torso
};

b2Body *build_body(b2World &world);

extern const std::array<b2Vec2, 3> sensor;
extern const float sensor_fov;
extern const float sensor_length;
extern const float sensor_width;

};

#endif
