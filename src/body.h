#ifndef BODY_H
#define BODY_H

#include <Box2D/Box2D.h>

namespace evsim {

b2Body *build_body(b2World &world);
b2Body *build_predator_body(b2World &world);

extern std::array<b2Vec2, 3> sensor;
extern float sensor_fov;
extern float sensor_length;
extern float sensor_width;

}

#endif
