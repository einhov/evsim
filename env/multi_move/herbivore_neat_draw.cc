#include <memory>
#include <vector>

#include <GL/glew.h>
#include <Box2D/Box2D.h>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../../evsim.h"
#include "../../body.h"
#include "../../gfx_program.h"

#include "herbivore_neat.h"

namespace evsim {
namespace multi_move {

static const std::string load_text_file(std::string_view filename) {
	std::ifstream file(filename.data());
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

static struct {
	struct { GLuint torso, sensor; } vertex_arrays;
	GLuint sensor_texture;
	std::unique_ptr<gfx::program> program;
	std::unique_ptr<gfx::program> program_sensor;
	bool hot;

	void init(int vision_segments) {
		static const std::vector<GLfloat> torso_verts {
			-1.0, -1.0,  1.0, -1.0,
			-1.0,  1.0,  1.0,  1.0
		};

		static const std::array<GLfloat, 6> sensor_uv{{
			0.0f, 0.001f,  -1.0f, 1.0f,  1.0f, 1.0f
		}};

		struct { GLuint torso, sensor, sensor_uv; } buffers;
		glGenVertexArrays(2, &vertex_arrays.torso);
		glGenBuffers(3, &buffers.torso);

		glBindVertexArray(vertex_arrays.torso);
		glBindBuffer(GL_ARRAY_BUFFER, buffers.torso);
		glBufferData(GL_ARRAY_BUFFER, torso_verts.size() * sizeof(GLfloat), torso_verts.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const void*>(0));
		glEnableVertexAttribArray(0);

		glBindVertexArray(vertex_arrays.sensor);
		glBindBuffer(GL_ARRAY_BUFFER, buffers.sensor);
		glBufferData(GL_ARRAY_BUFFER, sensor.size() * sizeof(sensor[0]), sensor.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const void*>(0));
		glBindBuffer(GL_ARRAY_BUFFER, buffers.sensor_uv);
		glBufferData(GL_ARRAY_BUFFER, sensor_uv.size() * sizeof(sensor_uv[0]), sensor_uv.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const void*>(0));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
		glDeleteBuffers(3, &buffers.torso);

		glGenTextures(1, &sensor_texture);
		glBindTexture(GL_TEXTURE_1D, sensor_texture);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, vision_segments, 0, GL_RED, GL_FLOAT, nullptr);

		program = std::make_unique<gfx::program>();
		program->attach(load_text_file("../box.vert"), gfx::program::shader_type::VERTEX);
		program->attach(load_text_file("../box.frag"), gfx::program::shader_type::FRAGMENT);
		program->link();

		program_sensor = std::make_unique<gfx::program>();
		program_sensor->attach(load_text_file("../sensor.vert"), gfx::program::shader_type::VERTEX);
		program_sensor->attach(load_text_file("../sensor.frag"), gfx::program::shader_type::FRAGMENT);
		program_sensor->link();

		hot = true;
	}

} model;

void herbivore_neat::draw(const glm::mat4 &projection) const {
	using uniform_type = gfx::program::uniform_type;

	if(!model.hot) model.init(agent::vision_segments);

	// Draw sensors
	if(state.draw_sensors_herbivore) {
		model.program_sensor->activate();
		model.program_sensor->set_uniform<uniform_type::MAT4>("projection", glm::value_ptr(projection));
		glBindVertexArray(model.vertex_arrays.sensor);
		glEnable(GL_TEXTURE_1D);
		glBindTexture(GL_TEXTURE_1D, model.sensor_texture);
		for(const auto &agent : agents) {
			if(!agent.active) continue;
			const auto body = agent.body;
			const b2Vec2 pos = body->GetPosition();
			const float angle = body->GetAngle();
			const glm::mat4 mat_model =
				glm::translate(glm::vec3(pos.x, pos.y, 0.0f)) *
				glm::rotate(angle, glm::vec3(0.0f, 0.0f, 1.0f));
			model.program_sensor->set_uniform<uniform_type::MAT4>("model", glm::value_ptr(mat_model));
			glTexSubImage1D(GL_TEXTURE_1D, 0, 0, agent::vision_segments, GL_RED, GL_FLOAT, agent.vision_predator.data());
			glGenerateMipmap(GL_TEXTURE_1D);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
		}
		glDisable(GL_TEXTURE_1D);
	}

	// Draw torsi
	model.program->activate();
	model.program->set_uniform<uniform_type::MAT4>("projection", glm::value_ptr(projection));
	glBindVertexArray(model.vertex_arrays.torso);
	for(const auto &agent : agents) {
		if(!agent.active) continue;
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
		const auto &c = colours[agent.internal_species % colours.size()];
		model.program->set_uniform<uniform_type::FLOAT3>("box_colour", c.x / 255.0f, c.y / 255.0f, c.z / 255.0f);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
}

}
}
