#include <random>

#include <glm/glm.hpp>
#include <Box2D/Box2D.h>

#include "body.h"

namespace evsim {

const float sensor_fov = glm::radians(60.0f);
const float sensor_length = 25.0f;
const float sensor_width = 2.0f * (sin(sensor_fov / 2.0f) * sensor_length) / cos(sensor_fov / 2.0f);

const std::array<b2Vec2, 3> sensor {{
	{ 0.0f, 0.0f }, { -sensor_width / 2.0f, sensor_length }, { sensor_width / 2.0f, sensor_length }
}};

static std::default_random_engine generator;
static std::uniform_real_distribution<float> velocity_distribution(-10.0f, 10.0f);
static std::uniform_real_distribution<float> angular_distribution(-glm::radians(45.0f), glm::radians(45.0f));
static std::uniform_real_distribution<float> pos_x_distribution(-99.0f * (4.0f / 3.0f), 99.0f * (4.0f / 3.0f));
static std::uniform_real_distribution<float> pos_y_distribution(-99.0f, 99.0f);

struct body_part_def {
	b2FixtureDef fixture;
	b2PolygonShape shape;
};

static body_part_def sensor_def;
static body_part_def torso_def;
static b2BodyDef body_def;

static bool body_initialised;

static const fixture_type sensor_type = fixture_type::sensor;
static const fixture_type torso_type = fixture_type::torso;

void init_body_data() {
	sensor_def.shape.Set(sensor.data(), sensor.size());
	sensor_def.fixture.density = 0.0f;
	sensor_def.fixture.shape = &sensor_def.shape;
	sensor_def.fixture.isSensor = true;
	sensor_def.fixture.filter.groupIndex = -1;
	sensor_def.fixture.userData = const_cast<void*>(static_cast<const void*>(&sensor_type));

	torso_def.shape.SetAsBox(1.0f, 1.0f);
	torso_def.fixture.shape = &torso_def.shape;
	torso_def.fixture.density = 1.0f;
	torso_def.fixture.filter.groupIndex = -1;
	torso_def.fixture.userData = const_cast<void*>(static_cast<const void*>(&torso_type));

	body_def.type = b2_dynamicBody;
	body_def.linearDamping = 10.0f;
	body_def.angularDamping = 10.0f;
	body_def.position.Set(pos_x_distribution(generator), pos_y_distribution(generator));
	body_def.linearVelocity = b2Vec2(velocity_distribution(generator), velocity_distribution(generator));
	body_def.angularVelocity = angular_distribution(generator);

	body_initialised = true;
}

b2Body *build_body(b2World &world) {
	if(!body_initialised)
		init_body_data();

	auto body = world.CreateBody(&body_def);
	body->CreateFixture(&sensor_def.fixture);
	body->CreateFixture(&torso_def.fixture);
	return body;
}

};
