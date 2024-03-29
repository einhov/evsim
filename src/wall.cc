#include <fstream>
#include <string>
#include <sstream>
#include <random>

#include <GL/glew.h>
#include <Box2D/Box2D.h>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "wall.h"
#include "fixture_type.h"
#include "gfx_program.h"
#include "evsim.h"
#include "entity.h"
#include "collision_data.h"

namespace evsim {

static const fixture_type fixture_type_wall = fixture_type::wall;
static const fixture_type fixture_type_wall_goal = fixture_type::wall_goal;
static const fixture_type fixture_type_wall_button = fixture_type::wall_button;

void wall::init_body(b2World &world, b2Vec2& position, b2Vec2& scale, wall_type type) {
	b2PolygonShape shape;
	shape.SetAsBox(scale.x, scale.y);
	b2FixtureDef fixture;

	fixture.shape = &shape;
	fixture.density = 1.0f;
	fixture.isSensor = false;
	this->type = type;
	if(type == wall_type::right) {
		fixture.filter.categoryBits = static_cast<uint16>(collision_types::WALL_RIGHT);
		fixture.userData = const_cast<void*>(static_cast<const void*>(&fixture_type_wall));
	} else if(type == wall_type::goal){
		fixture.filter.categoryBits = static_cast<uint16>(collision_types::WALL_GOAL);
		fixture.userData = const_cast<void*>(static_cast<const void*>(&fixture_type_wall_goal));
	} else if(type == wall_type::standard){
		fixture.filter.categoryBits = static_cast<uint16>(collision_types::WALL);
		fixture.userData = const_cast<void*>(static_cast<const void*>(&fixture_type_wall));
	} else if(type == wall_type::button){
		fixture.filter.categoryBits = static_cast<uint16>(collision_types::WALL_BUTTON);
		fixture.isSensor = true;
		fixture.userData = const_cast<void*>(static_cast<const void*>(&fixture_type_wall_button));
	} else {
		throw std::runtime_error(
			"Unknown wall_type sent to init_body (wall)"
		);
	}


	b2BodyDef def;
	def.type = b2_staticBody;
	def.position.Set(position.x, position.y);
	body = world.CreateBody(&def);
	body->CreateFixture(&fixture);
	body->SetUserData(this);

	this->world = &world;
	this->scale = scale;
}

void wall::set_active(bool active) {
	body->SetActive(active);
	this->active = active;
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

void wall::tick() {
}

void wall::draw(const glm::mat4 &projection) const {
	if(state.draw_wall) {
		if(!active) {
			return;
		}
		using uniform_type = gfx::program::uniform_type;
		if(!model.hot) model.init();
		model.program->activate();
		model.program->set_uniform<uniform_type::MAT4>("projection", glm::value_ptr(projection));
		const b2Vec2 pos = body->GetPosition();
		const glm::mat4 mat_model =
			glm::translate(glm::vec3(pos.x, pos.y, 0.0f)) * glm::scale(glm::vec3(scale.x, scale.y, 1.0f));
		glBindVertexArray(model.vertex_arrays.body);
		model.program->set_uniform<uniform_type::MAT4>("model", glm::value_ptr(mat_model));
		if(type == wall_type::goal) {
			model.program->set_uniform<uniform_type::FLOAT3>("box_colour", 0.0f, 0.8f, 0.0f);
		}
		else if(type == wall_type::button) {
			model.program->set_uniform<uniform_type::FLOAT3>("box_colour", 0.7f, 0.5f, 0.0f);
		}
		else {
			model.program->set_uniform<uniform_type::FLOAT3>("box_colour", 1.0f, 0.5f, 0.25f);
		}
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
}

wall::~wall() {
	world->DestroyBody(this->body);
}

}
