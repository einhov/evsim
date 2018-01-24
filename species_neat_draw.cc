#include <memory>
#include <vector>

#include <GL/glew.h>
#include <Box2D/Box2D.h>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "species_neat.h"
#include "body.h"
#include "gfx_program.h"

namespace evsim {

static const std::string load_text_file(std::string_view filename) {
	std::ifstream file(filename.data());
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

static struct {
	struct { GLuint torso, sensor_left, sensor_right; } vertex_arrays;
	std::unique_ptr<gfx::program> program;
	bool hot;

	void init() {
		static const std::vector<GLfloat> torso_verts {
			-1.0, -1.0,  1.0, -1.0,
			-1.0,  1.0,  1.0,  1.0
		};

		struct { GLuint torso, sensor_left, sensor_right; } buffers;
		glGenVertexArrays(3, &vertex_arrays.torso);
		glGenBuffers(3, &buffers.torso);

		glBindVertexArray(vertex_arrays.torso);
		glBindBuffer(GL_ARRAY_BUFFER, buffers.torso);
		glBufferData(GL_ARRAY_BUFFER, torso_verts.size() * sizeof(GLfloat), torso_verts.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const void*>(0));
		glEnableVertexAttribArray(0);

		glBindVertexArray(vertex_arrays.sensor_left);
		glBindBuffer(GL_ARRAY_BUFFER, buffers.sensor_left);
		glBufferData(GL_ARRAY_BUFFER, sensor_left.size() * sizeof(sensor_left[0]), sensor_left.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const void*>(0));
		glEnableVertexAttribArray(0);

		glBindVertexArray(vertex_arrays.sensor_right);
		glBindBuffer(GL_ARRAY_BUFFER, buffers.sensor_right);
		glBufferData(GL_ARRAY_BUFFER, sensor_right.size() * sizeof(sensor_right[0]), sensor_right.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const void*>(0));
		glEnableVertexAttribArray(0);

		glBindVertexArray(0);
		glDeleteBuffers(3, &buffers.torso);

		program = std::make_unique<gfx::program>();
		program->attach(load_text_file("../box.vert"), gfx::program::shader_type::VERTEX);
		program->attach(load_text_file("../box.frag"), gfx::program::shader_type::FRAGMENT);
		program->link();

		hot = true;
	}

} model;

void species_neat::draw(const glm::mat4 &projection) const {
	using uniform_type = gfx::program::uniform_type;

	if(!model.hot) model.init();
	model.program->activate();
	model.program->set_uniform<uniform_type::MAT4>("projection", glm::value_ptr(projection));

	// Draw sensors
	constexpr bool render_sensors = true;
	if constexpr(render_sensors) {
		for(const auto &agent : agents) {
			const auto body = agent.body;
			const b2Vec2 pos = body->GetPosition();
			const float angle = body->GetAngle();
			const glm::mat4 mat_model =
				glm::translate(glm::vec3(pos.x, pos.y, 0.0f)) *
				glm::rotate(angle, glm::vec3(0.0f, 0.0f, 1.0f));
			model.program->set_uniform<uniform_type::MAT4>("model", glm::value_ptr(mat_model));

			if(!agent.detected[0]) {
				model.program->set_uniform<uniform_type::FLOAT3>("box_colour", 0.0f, 1.0f, 0.0f);
			} else {
				model.program->set_uniform<uniform_type::FLOAT3>("box_colour", 1.0f, 0.0f, 0.0f);
			}
			glBindVertexArray(model.vertex_arrays.sensor_left);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);

			if(!agent.detected[1]) {
				model.program->set_uniform<uniform_type::FLOAT3>("box_colour", 0.0f, 1.0f, 0.0f);
			} else {
				model.program->set_uniform<uniform_type::FLOAT3>("box_colour", 1.0f, 0.0f, 0.0f);
			}
			glBindVertexArray(model.vertex_arrays.sensor_right);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
		}
	}

	// Draw torsi
	glBindVertexArray(model.vertex_arrays.torso);
	for(const auto &agent : agents) {
		const auto box = agent.body;
		const b2Vec2 pos = box->GetPosition();
		const float angle = box->GetAngle();
		const b2Vec2 vel = box->GetLinearVelocity();
		const glm::mat4 mat_model =
			glm::translate(glm::vec3(pos.x, pos.y, 0.0f)) *
			glm::rotate(angle, glm::vec3(0.0f, 0.0f, 1.0f));
		model.program->set_uniform<uniform_type::MAT4>("model", glm::value_ptr(mat_model));
		static const std::array<glm::vec3, 21> colours {{
			{ 64,255,115 }, { 204,51,92 }, { 242,129,0 },
			{ 0,29,217 }, { 0,109,204 }, { 64,255,242 },
			{ 204,51,133 }, { 0,162,242 }, { 217,0,202 },
			{ 64,217,255 }, { 255,238,0 }, { 0,242,162 },
			{ 217,145,0 }, { 172,57,230 }, { 242,65,0 },
			{ 229,184,0 }, { 0,204,163 }, { 229,57,57 },
			{ 97,242,0 }, { 184,230,0 }, { 0,82,204 }
		}};
		const auto &c = colours[agent.species % colours.size()];
		model.program->set_uniform<uniform_type::FLOAT3>("box_colour", c.x / 255.0f, c.y / 255.0f, c.z / 255.0f);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
}

};
