#include <random>

#include <glm/glm.hpp>
#include <Box2D/Box2D.h>

#include "sim.h"

namespace evsim {

static std::default_random_engine generator;
static std::uniform_real_distribution<float> velocity_distribution(-10.0f, 10.0f);
static std::uniform_real_distribution<float> angular_distribution(-glm::radians(45.0f), glm::radians(45.0f));
static std::uniform_real_distribution<float> pos_x_distribution(-99.0f * (4.0f / 3.0f), 99.0f * (4.0f / 3.0f));
static std::uniform_real_distribution<float> pos_y_distribution(-99.0f, 99.0f);

void food::init_body(b2World &world) {
	b2PolygonShape shape;
	shape.SetAsBox(0.5f, 0.5f);
	b2FixtureDef fixture;
	fixture.shape = &shape;
	fixture.density = 1.0f;
	fixture.filter.groupIndex = 0;
	fixture.isSensor = true;
	fixture.userData = reinterpret_cast<void*>(0x900df00d);

	b2BodyDef def;
	def.type = b2_staticBody;
	def.position.Set(pos_x_distribution(generator), pos_y_distribution(generator));
	body = world.CreateBody(&def);
	body->CreateFixture(&fixture);
}

void food::update() {
	const auto pos = body->GetPosition();
	const auto angle = body->GetAngle();
	if(pos.y < -100.0f) body->SetTransform(b2Vec2(pos.x, 100.0f), angle);
	if(pos.y > 100.0f) body->SetTransform(b2Vec2(pos.x, -100.0f), angle);
	if(pos.x < -100.0f * (4.0 / 3.0)) body->SetTransform(b2Vec2(100.0f * (4.0 / 3.0), pos.y), angle);
	if(pos.x > 100.0f * (4.0 / 3.0)) body->SetTransform(b2Vec2(-100.0f * (4.0 / 3.0), pos.y), angle);
}

};
