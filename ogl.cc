#include <cmath>
#include <random>
#include <fstream>
#include <string>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <Box2D/Box2D.h>

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
	prog.attach(load_text_file("../box.vert"), gfx::program::shader_type::VERTEX);
	prog.attach(load_text_file("../box.frag"), gfx::program::shader_type::FRAGMENT);
	prog.link();

	gfx::program sensor_prog;
	sensor_prog.attach(load_text_file("../sensor.vert"), gfx::program::shader_type::VERTEX);
	sensor_prog.attach(load_text_file("../sensor.frag"), gfx::program::shader_type::FRAGMENT);
	sensor_prog.link();

	gfx::program black_overlay;
	black_overlay.attach(load_text_file("../2d_passthrough.vert"), gfx::program::shader_type::VERTEX);
	black_overlay.attach(load_text_file("../black.frag"), gfx::program::shader_type::FRAGMENT);
	black_overlay.link();

	static const std::vector<GLfloat> rectangle_verts {
		-1.0, -1.0, 0.0, 0.0,  1.0, -1.0, 1.0, 0.0,
		-1.0,  1.0, 0.0, 1.0,  1.0,  1.0, 1.0, 1.0
	};

	static const std::array<b2Vec2, 3> sensor_vertices {{
		{ 0.0f, 0.0f }, { 2.0f, 20.0f }, { -2.0f, 20.0f },
	}};

	std::default_random_engine generator;
	std::uniform_real_distribution<float> velocity_distribution(-10.0f, 10.0f);
	std::uniform_real_distribution<float> angular_distribution(-glm::radians(1.0f), glm::radians(1.0f));
	std::uniform_real_distribution<float> position_distribution(-100.0f, 100.0f);

	struct { GLuint rectangle, sensor; } vertex_arrays;
	{
		struct { GLuint rectangle_vertex, rectangle_colour, sensor_vertex, position; } buffers;
		glGenVertexArrays(2, &vertex_arrays.rectangle);
		glGenBuffers(4, &buffers.rectangle_vertex);

		glBindVertexArray(vertex_arrays.rectangle);
		glBindBuffer(GL_ARRAY_BUFFER, buffers.rectangle_vertex);
		glBufferData(GL_ARRAY_BUFFER, rectangle_verts.size() * sizeof(GLfloat), rectangle_verts.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), reinterpret_cast<const void*>(0));
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), reinterpret_cast<const void*>(2 * sizeof(GLfloat)));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glBindVertexArray(vertex_arrays.sensor);
		glBindBuffer(GL_ARRAY_BUFFER, buffers.sensor_vertex);
		glBufferData(GL_ARRAY_BUFFER, sensor_vertices.size() * sizeof(sensor_vertices[0]), sensor_vertices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const void*>(0));
		glEnableVertexAttribArray(0);

		glBindVertexArray(0);
		glDeleteBuffers(4, &buffers.rectangle_vertex);
	}

	const glm::mat4 projection = glm::ortho(-100.0f * (4.0f / 3.0f), 100.0f * (4.0f / 3.0f), -100.0f, 100.f);
	using uniform_type = gfx::program::uniform_type;
	const auto b1 = prog.set_uniform<uniform_type::MAT4>("projection", glm::value_ptr(projection));

	const float simulation_timestep = 1.0f/60.0f;

	b2World world(b2Vec2(0.0f, 0.0f));

	b2PolygonShape sensorShape;
	sensorShape.Set(sensor_vertices.data(), sensor_vertices.size());
	b2FixtureDef sensorFixtDef;
	sensorFixtDef.density = 0.0f;
	sensorFixtDef.shape = &sensorShape;
	sensorFixtDef.isSensor = true;
	sensorFixtDef.filter.groupIndex = -1;

	b2PolygonShape boxBox;
	boxBox.SetAsBox(1.0f, 1.0f);
	b2FixtureDef boxFixtDef;
	boxFixtDef.shape = &boxBox;
	boxFixtDef.density = 1.0f;

	static constexpr size_t AGENTS = 128;

	struct agent {
		b2Body *body;
	};
	std::array<agent, AGENTS> agents;

	for(auto &agent : agents) {
		b2BodyDef boxDef;
		boxDef.type = b2_dynamicBody;
		boxDef.linearDamping = 0.1f;
		boxDef.angularDamping = 0.5f;
		boxDef.position.Set(position_distribution(generator), position_distribution(generator));
		boxDef.linearVelocity = b2Vec2(velocity_distribution(generator), velocity_distribution(generator));
		boxDef.angularVelocity = angular_distribution(generator);
		b2Body *box = world.CreateBody(&boxDef);
		box->CreateFixture(&boxFixtDef);
		box->CreateFixture(&sensorFixtDef);
		agent = { box };
	}

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	double previous_frame = glfwGetTime();
	while(true) {
		const double this_frame = glfwGetTime();
		const double delta = this_frame - previous_frame;

		glEnable(GL_BLEND);
		black_overlay.activate();
		black_overlay.set_uniform<uniform_type::FLOAT>("alpha", static_cast<float>(delta) * 5.0f);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glDisable(GL_BLEND);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		world.Step(simulation_timestep, 10, 10);

		// Sensors
		{
			sensor_prog.activate();
			glBindVertexArray(vertex_arrays.sensor);

			for(const auto &agent : agents) {
				auto &box = agent.body;
				const b2Vec2 pos = box->GetPosition();
				const float angle = box->GetAngle();
				const glm::mat4 model =
					glm::translate(glm::vec3(pos.x, pos.y, 0.0f)) *
					glm::rotate(angle, glm::vec3(0.0f, 0.0f, 1.0f));
				prog.set_uniform<uniform_type::MAT4>("model", glm::value_ptr(model));
				bool detected = false;
				for(const b2ContactEdge *edge = box->GetContactList(); edge != nullptr && !detected; edge = edge->next) {
					const auto contact = edge->contact;
					const b2Fixture * fixture = contact->GetFixtureA();
					if(fixture->GetBody() != box) fixture = contact->GetFixtureB();
					if(fixture->IsSensor() && contact->IsTouching()) {
						detected = true;
					}
				}
				if(!detected) {
					prog.set_uniform<uniform_type::FLOAT3>("box_colour", 0.0f, 1.0f, 0.0f);
				} else {
					prog.set_uniform<uniform_type::FLOAT3>("box_colour", 1.0f, 0.0f, 0.0f);
					const auto dir = glm::rotate(glm::vec2 { 0.0f, 1.0f }, angle) * 100.0f;
					box->ApplyForceToCenter(b2Vec2 { dir.x, dir.y }, true);
				}

				prog.activate();
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
			}
		}

		// Agents
		{
			prog.activate();
			glBindVertexArray(vertex_arrays.rectangle);

			for(const auto &agent : agents) {
				auto &box = agent.body;
				const b2Vec2 pos = box->GetPosition();
				const float angle = box->GetAngle();
				const b2Vec2 vel = box->GetLinearVelocity();
				const glm::mat4 model =
					glm::translate(glm::vec3(pos.x, pos.y, 0.0f)) *
					glm::rotate(angle, glm::vec3(0.0f, 0.0f, 1.0f));
				prog.set_uniform<uniform_type::MAT4>("model", glm::value_ptr(model));
				const glm::vec3 c = glm::mix(
					glm::vec3(1.0f, 0.0f, 0.0f),
					glm::vec3(0.0f, 1.0f, 0.0f),
					glm::clamp(glm::length(glm::vec2(vel.x, vel.y)) / 100.0f, 0.0f, 1.0f)
				);
				prog.set_uniform<uniform_type::FLOAT3>("box_colour", c.x, c.y, c.z);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

				if(pos.y < -100.0f) box->SetTransform(b2Vec2(pos.x, 100.0f), angle);
				if(pos.y > 100.0f) box->SetTransform(b2Vec2(pos.x, -100.0f), angle);
				if(pos.x < -100.0f * (4.0 / 3.0)) box->SetTransform(b2Vec2(100.0f * (4.0 / 3.0), pos.y), angle);
				if(pos.x > 100.0f * (4.0 / 3.0)) box->SetTransform(b2Vec2(-100.0f * (4.0 / 3.0), pos.y), angle);
			}
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
		previous_frame = this_frame;
	}

	glfwTerminate();
	return 0;
}
