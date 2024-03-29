#include <fstream>
#include <string>
#include <sstream>
#include <random>

#include <GL/glew.h>
#include <Box2D/Box2D.h>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "consumable.h"
#include "fixture_type.h"
#include "gfx_program.h"
#include "collision_data.h"

namespace evsim {

static std::default_random_engine generator(std::random_device{}());
static std::uniform_real_distribution<float> velocity_distribution(-10.0f, 10.0f);
static std::uniform_real_distribution<float> pos_x_distribution(-500.0f * (4.0f / 3.0f), 500.0f * (4.0f / 3.0f));
static std::uniform_real_distribution<float> pos_y_distribution(-500.0f, 500.0f);
static std::uniform_real_distribution<float> rotation_distribution(0.0f, glm::radians(360.0f));

static const fixture_type food_type = fixture_type::food;

void consumable::init_body(b2World &world) {
	b2PolygonShape shape;
	shape.SetAsBox(0.5f, 0.5f);
	b2FixtureDef fixture;
	fixture.shape = &shape;
	fixture.density = 1.0f;
	fixture.filter.categoryBits = static_cast<uint16>(collision_types::CONSUMABLE);
	fixture.filter.maskBits =
			static_cast<uint16>(collision_types::HERBIVORE) |
			static_cast<uint16>(collision_types::SENSOR);
	fixture.isSensor = true;
	fixture.userData = const_cast<void*>(static_cast<const void*>(&food_type));

	b2BodyDef def;
	def.type = b2_staticBody;
	def.position.Set(pos_x_distribution(generator), pos_y_distribution(generator));
	body = world.CreateBody(&def);
	body->CreateFixture(&fixture);
	body->SetUserData(this);
}

static void relocate_consumable(b2Body *body) {
	body->SetTransform(b2Vec2(pos_x_distribution(generator), pos_y_distribution(generator)), 0.0f);
}

void consumable::pre_step() {
	relocate_consumable(body);
}

void consumable::message(const std::any &msg) {
	const auto &type = msg.type();
	if(type == typeid(msg_consume)) {
		const auto &consumer = std::any_cast<msg_consume>(msg).consumer;
		consumer->message(std::make_any<msg_consumed>());
		relocate_consumable(body);
	}
}

static const std::string load_text_file(std::string_view filename) {
	std::ifstream file(filename.data());
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

static struct {
	struct { GLuint body; } vertex_arrays;
	std::unique_ptr<gfx::program> program;
	bool hot;

	void init() {
		static const std::vector<GLfloat> body_vertices {
			-1.0, -1.0,   1.0, -1.0,
			-1.0,  1.0,   1.0,  1.0
		};

		struct { GLuint body; } buffers;
		glGenVertexArrays(1, &vertex_arrays.body);
		glGenBuffers(1, &buffers.body);

		glBindVertexArray(vertex_arrays.body);
		glBindBuffer(GL_ARRAY_BUFFER, buffers.body);
		glBufferData(GL_ARRAY_BUFFER, body_vertices.size() * sizeof(GLfloat), body_vertices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const void*>(0));
		glEnableVertexAttribArray(0);

		glBindVertexArray(0);
		glDeleteBuffers(1, &buffers.body);

		program = std::make_unique<gfx::program>();
		program->attach(load_text_file("shaders/box.vert"), gfx::program::shader_type::VERTEX);
		program->attach(load_text_file("shaders/box.frag"), gfx::program::shader_type::FRAGMENT);
		program->link();
		hot = true;
	}

} model;

void consumable::draw(const glm::mat4 &projection) const {
	using uniform_type = gfx::program::uniform_type;

	if(!model.hot) model.init();
	model.program->activate();
	model.program->set_uniform<uniform_type::MAT4>("projection", glm::value_ptr(projection));

	const b2Vec2 pos = body->GetPosition();
	const glm::mat4 mat_model =
		glm::translate(glm::vec3(pos.x, pos.y, 0.0f)) * glm::scale(glm::vec3(0.5f));
	glBindVertexArray(model.vertex_arrays.body);
	model.program->set_uniform<uniform_type::MAT4>("model", glm::value_ptr(mat_model));
	model.program->set_uniform<uniform_type::FLOAT3>("box_colour", 0.0f, 0.0f, 1.0f);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

}
