#include <fstream>
#include <string>
#include <sstream>
#include <random>

#include <GL/glew.h>
#include <Box2D/Box2D.h>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "yell.h"
#include "fixture_type.h"
#include "gfx_program.h"
#include "evsim.h"
#include "entity.h"
#include "collision_data.h"

namespace evsim {

static const fixture_type yell_type = fixture_type::yell;

void yell::init_body(b2World &world, entity *hollerer, const b2Vec2& position) {
	b2CircleShape shape;
	shape.m_p.Set(0.0f, 0.0f); //position, relative to body position
	shape.m_radius = 30.0f;

	b2FixtureDef fixture;
	fixture.shape = &shape;
	fixture.density = 1.0f;
	fixture.filter.groupIndex = 0;
	fixture.isSensor = true;
	fixture.filter.categoryBits = static_cast<uint16>(collision_types::YELL);
	fixture.filter.maskBits = static_cast<uint16>(collision_types::HERBIVORE);
	fixture.userData = const_cast<void*>(static_cast<const void*>(&yell_type));

	b2BodyDef def;
	def.type = b2_staticBody;
	def.position.Set(position.x, position.y);
	body = world.CreateBody(&def);
	body->CreateFixture(&fixture);
	body->SetUserData(this);

	this->hollerer = hollerer;
	this->world = &world;
}

static const std::string load_text_file(std::string_view filename) {
	std::ifstream file(filename.data());
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

void yell::schedule_stop_yell() {
	to_be_destroyed.push_back(this);
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
		program->attach(load_text_file("shaders/circle.vert"), gfx::program::shader_type::VERTEX);
		program->attach(load_text_file("shaders/circle.frag"), gfx::program::shader_type::FRAGMENT);
		program->link();
		hot = true;
	}
} model;

void yell::tick() {
	ticks_to_live--;
	if(ticks_to_live == 0) {
		schedule_stop_yell();
	}
}

void yell::pre_step() {
	schedule_stop_yell();
}

void yell::draw(const glm::mat4 &projection) const {
	if(state.draw_yell) {
		using uniform_type = gfx::program::uniform_type;
		if(!model.hot) model.init();
		model.program->activate();
		model.program->set_uniform<uniform_type::MAT4>("projection", glm::value_ptr(projection));

		const b2Vec2 pos = body->GetPosition();
		const glm::mat4 mat_model =
			glm::translate(glm::vec3(pos.x, pos.y, 0.0f)) * glm::scale(glm::vec3(30.0f));
		glBindVertexArray(model.vertex_arrays.body);
		model.program->set_uniform<uniform_type::MAT4>("model", glm::value_ptr(mat_model));
		model.program->set_uniform<uniform_type::FLOAT3>("box_colour", 1.0f, 0.5f, 0.25f);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
}

yell::~yell() {
	world->DestroyBody(this->body);
}

}
