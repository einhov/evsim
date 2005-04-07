#include <cmath>
#include <random>
#include <fstream>
#include <string>
#include <sstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "gfx_program.h"

const std::string load_text_file(std::string_view filename) {
	std::ifstream file(filename.data());
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

int main(int argc, char **argv) {
	if(!glfwInit()) {
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *window = glfwCreateWindow(1024, 768, "", nullptr, nullptr);
	if(!window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glewExperimental = true;

	if(glewInit() != GLEW_OK) {
		return -1;
	}

	gfx::program prog;
	prog.attach(load_text_file("../particle.vert"), gfx::program::shader_type::VERTEX);
	prog.attach(load_text_file("../particle.frag"), gfx::program::shader_type::FRAGMENT);
	prog.link();

	gfx::program black_overlay;
	black_overlay.attach(load_text_file("../2d_passthrough.vert"), gfx::program::shader_type::VERTEX);
	black_overlay.attach(load_text_file("../black.frag"), gfx::program::shader_type::FRAGMENT);
	black_overlay.link();

	static const std::vector<GLfloat> rectangle_verts {
		-1.0, -1.0, 0.0, 0.0,  1.0, -1.0, 1.0, 0.0,
		-1.0,  1.0, 0.0, 1.0,  1.0,  1.0, 1.0, 1.0
	};

	std::default_random_engine generator;
	std::uniform_real_distribution<float> velocity_distribution(-10.0f, 10.0f);
	std::uniform_real_distribution<float> position_distribution(-100.0f, 100.0f);
	std::uniform_real_distribution<float> colour_distribution(0.0f, 1.0f);

	static constexpr size_t sprites = 5000;

	std::array<glm::vec2, sprites> sprite_velocity;
	for(auto &vel : sprite_velocity) vel = { velocity_distribution(generator), velocity_distribution(generator) };
	std::array<glm::vec2, sprites> sprite_positions;
	for(auto &pos : sprite_positions) pos = { position_distribution(generator), position_distribution(generator) };
	std::array<glm::vec3, sprites> sprite_colours;
	for(auto &col : sprite_colours) col = { colour_distribution(generator), colour_distribution(generator), colour_distribution(generator) };

	GLuint vao, vbo, positions, colours;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &positions);
	glGenBuffers(1, &colours);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, rectangle_verts.size() * sizeof(GLfloat), rectangle_verts.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), reinterpret_cast<const void*>(0));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), reinterpret_cast<const void*>(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, positions);
	glBufferData(GL_ARRAY_BUFFER, sprite_positions.size() * sizeof(sprite_positions[0]), nullptr, GL_STREAM_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glVertexAttribDivisor(2, 1);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, colours);
	glBufferData(GL_ARRAY_BUFFER, sprite_colours.size() * sizeof(sprite_colours[0]), sprite_colours.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glVertexAttribDivisor(3, 1);
	glEnableVertexAttribArray(3);
	glBindVertexArray(0);
	glDeleteBuffers(1, &vbo);
	glBindVertexArray(vao);

	const glm::mat4 projection = glm::ortho(-100.0f * (4.0f / 3.0f), 100.0f * (4.0f / 3.0f), -100.0f, 100.f);
	using uniform_type = gfx::program::uniform_type;
	const auto b1 = prog.set_uniform<uniform_type::MAT4>("projection", glm::value_ptr(projection));

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	double previous_frame = glfwGetTime();
	while(true) {
		const double this_frame = glfwGetTime();
		const double delta = this_frame - previous_frame;
		printf("Frame time: %.3fms (%.1f FPS)\n", delta * 1000.0, 1.0/delta);
		glEnable(GL_BLEND);
		black_overlay.activate();
		black_overlay.set_uniform<uniform_type::FLOAT>("alpha", static_cast<float>(delta) * 5.0f);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glDisable(GL_BLEND);

		prog.activate();
		prog.set_uniform<uniform_type::FLOAT>("test", static_cast<GLfloat>(sin(glfwGetTime())));
		const auto instances = sprite_positions.size();
		for(size_t i = 0; i < instances; i++) {
			auto &pos = sprite_positions[i];
			auto &vel = sprite_velocity[i];

			const glm::vec2 diff = glm::vec2(0.0f) - pos;
			const float dist = glm::length(diff);
			const float accel = 10000.0f / std::pow(dist, 2.0f);

			vel += glm::normalize(diff) * accel * static_cast<float>(delta);
			pos += vel * static_cast<float>(delta);
			if(dist > 200.0f) {
				vel = { velocity_distribution(generator), velocity_distribution(generator) };
				pos = { position_distribution(generator), position_distribution(generator) };
				sprite_colours[i] = { colour_distribution(generator), colour_distribution(generator), colour_distribution(generator) };
			}
		}
		glBindBuffer(GL_ARRAY_BUFFER, positions);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sprite_positions.size() * sizeof(sprite_positions[0]), sprite_positions.data());
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, sprites);

		glfwSwapBuffers(window);
		glfwPollEvents();
		previous_frame = this_frame;
	}

	glfwTerminate();
	return 0;
}
