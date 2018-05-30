#include <random>

#include <glm/glm.hpp>
#include <Box2D/Box2D.h>

#include "fixture_type.h"
#include "body.h"
#include "config.h"
#include "collision_data.h"

namespace evsim {

static std::default_random_engine generator;
static std::uniform_real_distribution<float> velocity_distribution(-10.0f, 10.0f);
static std::uniform_real_distribution<float> angular_distribution(-glm::radians(45.0f), glm::radians(45.0f));
static std::uniform_real_distribution<float> pos_x_distribution(-99.0f * (4.0f / 3.0f), 99.0f * (4.0f / 3.0f));
static std::uniform_real_distribution<float> pos_y_distribution(-99.0f, 99.0f);

struct body_part_def {
	b2FixtureDef fixture;
	b2PolygonShape shape;
	b2CircleShape circle_shape;
};

static body_part_def sensor_def;
static body_part_def torso_def;
static body_part_def torso_predator_def;
static b2BodyDef body_def;

static bool body_initialised;

static const fixture_type sensor_type = fixture_type::sensor;
static const fixture_type torso_type = fixture_type::torso;
static const fixture_type torso_predator_type = fixture_type::torso_predator;

float sensor_fov = config::sensor_fov;
float sensor_length = config::sensor_length;
float sensor_width = 2.0f * (sin(sensor_fov / 2.0f) * sensor_length) / cos(sensor_fov / 2.0f);
std::array<b2Vec2, 3> sensor {};

void init_body_data() {
	sensor_fov = config::sensor_fov;
	sensor_length = config::sensor_length;
	sensor_width = 2.0f * (sin(sensor_fov / 2.0f) * sensor_length) / cos(sensor_fov / 2.0f);
	sensor = {{
		{ 0.0f, 0.0f }, { -sensor_width / 2.0f, sensor_length },
		{ sensor_width / 2.0f, sensor_length }
	}};

	sensor_def.shape.Set(sensor.data(), sensor.size());
	sensor_def.fixture.density = 0.0f;
	sensor_def.fixture.shape = &sensor_def.shape;
	sensor_def.fixture.isSensor = true;
	sensor_def.fixture.filter.categoryBits = static_cast<uint16>(collision_types::SENSOR);
	sensor_def.fixture.filter.maskBits =
		static_cast<uint16>(collision_types::HERBIVORE) |
		static_cast<uint16>(collision_types::CONSUMABLE) |
		static_cast<uint16>(collision_types::PREDATOR) |
		static_cast<uint16>(collision_types::WALL) |
		static_cast<uint16>(collision_types::WALL_GOAL) |
		static_cast<uint16>(collision_types::WALL_BUTTON)
	;
	sensor_def.fixture.userData = const_cast<void*>(static_cast<const void*>(&sensor_type));

	torso_def.circle_shape.m_p.Set(0.0f, 0.0f);
	torso_def.circle_shape.m_radius = 1.0f;
	torso_def.fixture.shape = &torso_def.circle_shape;
	torso_def.fixture.density = 1.0f;
	torso_def.fixture.isSensor = false;
	torso_def.fixture.filter.categoryBits = static_cast<uint16>(collision_types::HERBIVORE);
	torso_def.fixture.filter.maskBits =
		static_cast<uint16>(collision_types::PREDATOR) |
		static_cast<uint16>(collision_types::CONSUMABLE) |
		static_cast<uint16>(collision_types::SENSOR) |
		static_cast<uint16>(collision_types::WALL) |
		static_cast<uint16>(collision_types::WALL_RIGHT) |
		static_cast<uint16>(collision_types::WALL_GOAL) |
		static_cast<uint16>(collision_types::YELL) |
		static_cast<uint16>(collision_types::WALL_BUTTON)
	;
	torso_def.fixture.userData = const_cast<void*>(static_cast<const void*>(&torso_type));

	torso_predator_def.circle_shape.m_p.Set(0.0f, 0.0f);
	torso_predator_def.circle_shape.m_radius = 1.0f;
	torso_predator_def.fixture.shape = &torso_predator_def.circle_shape;
	torso_predator_def.fixture.density = 1.0f;
	torso_predator_def.fixture.isSensor = false;
	torso_predator_def.fixture.filter.categoryBits = static_cast<uint16>(collision_types::PREDATOR);
	torso_predator_def.fixture.filter.maskBits =
		static_cast<uint16>(collision_types::HERBIVORE) |
		static_cast<uint16>(collision_types::SENSOR) |
		static_cast<uint16>(collision_types::WALL) |
		static_cast<uint16>(collision_types::WALL_RIGHT)
	;
	torso_predator_def.fixture.userData = const_cast<void*>(static_cast<const void*>(&torso_predator_type));

	body_def.type = b2_dynamicBody;
	body_def.linearDamping = config::linear_damping;
	body_def.angularDamping = config::angular_damping;
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

b2Body *build_predator_body(b2World &world) {
	if(!body_initialised)
		init_body_data();

	auto body = world.CreateBody(&body_def);
	body->CreateFixture(&sensor_def.fixture);
	body->CreateFixture(&torso_predator_def.fixture);
	return body;
}

};
