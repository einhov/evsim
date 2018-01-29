#ifndef BODY_H
#define BODY_H

namespace evsim {

b2Body *build_body(b2World &world);
b2Body *build_predator_body(b2World &world);

extern const std::array<b2Vec2, 3> sensor_left;
extern const std::array<b2Vec2, 3> sensor_right;

};

#endif
