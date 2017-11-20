#include <cmath>
#include <random>
#include <fstream>
#include <string>
#include <sstream>
#include <cassert>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <Box2D/Box2D.h>

#include "gfx_program.h"

#include <Genome.h>
#include <Population.h>
#include <NeuralNetwork.h>
#include <Parameters.h>

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

	static constexpr bool trails = false;

	std::optional<gfx::program> black_overlay;
	if constexpr(trails) {
		black_overlay.emplace();
		black_overlay->attach(load_text_file("../2d_passthrough.vert"), gfx::program::shader_type::VERTEX);
		black_overlay->attach(load_text_file("../black.frag"), gfx::program::shader_type::FRAGMENT);
		black_overlay->link();
	}

	static const std::vector<GLfloat> rectangle_verts {
		-1.0, -1.0, 0.0, 0.0,  1.0, -1.0, 1.0, 0.0,
		-1.0,  1.0, 0.0, 1.0,  1.0,  1.0, 1.0, 1.0
	};

	static const std::array<b2Vec2, 3> sensor_vertices {{
		{ 0.0f, 0.0f }, { 1.0f, 200.0f }, { -1.0f, 200.0f },
	}};

	constexpr float sensor_length = 50.0f;
	constexpr float sensor_half_width = 2.0f;

	static const std::array<b2Vec2, 3> sensor_left {{
		{ 0.0f, 0.0f }, {-sensor_half_width, sensor_length }, { 0.0f, sensor_length },
	}};
	static const std::array<b2Vec2, 3> sensor_right {{
		{ 0.0f, 0.0f }, { 0.0f, sensor_length }, { sensor_half_width, sensor_length },
	}};

	static std::default_random_engine generator;
	static std::uniform_real_distribution<float> velocity_distribution(-10.0f, 10.0f);
	static std::uniform_real_distribution<float> angular_distribution(-glm::radians(45.0f), glm::radians(45.0f));
	static std::uniform_real_distribution<float> pos_x_distribution(-99.0f * (4.0f / 3.0f), 99.0f * (4.0f / 3.0f));
	static std::uniform_real_distribution<float> pos_y_distribution(-99.0f, 99.0f);

	struct { GLuint rectangle, sensor, sensorr; } vertex_arrays;
	{
		struct { GLuint rectangle_vertex, sensor_vertex, sensor_vertex_right; } buffers;
		glGenVertexArrays(3, &vertex_arrays.rectangle);
		glGenBuffers(3, &buffers.rectangle_vertex);

		glBindVertexArray(vertex_arrays.rectangle);
		glBindBuffer(GL_ARRAY_BUFFER, buffers.rectangle_vertex);
		glBufferData(GL_ARRAY_BUFFER, rectangle_verts.size() * sizeof(GLfloat), rectangle_verts.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), reinterpret_cast<const void*>(0));
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), reinterpret_cast<const void*>(2 * sizeof(GLfloat)));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glBindVertexArray(vertex_arrays.sensor);
		glBindBuffer(GL_ARRAY_BUFFER, buffers.sensor_vertex);
		glBufferData(GL_ARRAY_BUFFER, sensor_left.size() * sizeof(sensor_left[0]), sensor_left.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const void*>(0));
		glEnableVertexAttribArray(0);

		glBindVertexArray(vertex_arrays.sensorr);
		glBindBuffer(GL_ARRAY_BUFFER, buffers.sensor_vertex_right);
		glBufferData(GL_ARRAY_BUFFER, sensor_right.size() * sizeof(sensor_right[0]), sensor_right.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const void*>(0));
		glEnableVertexAttribArray(0);

		glBindVertexArray(0);
		glDeleteBuffers(3, &buffers.rectangle_vertex);
	}

	const glm::mat4 projection = glm::ortho(-100.0f * (4.0f / 3.0f), 100.0f * (4.0f / 3.0f), -100.0f, 100.f);
	using uniform_type = gfx::program::uniform_type;
	const auto b1 = prog.set_uniform<uniform_type::MAT4>("projection", glm::value_ptr(projection));

	const float simulation_timestep = 1.0f/60.0f;

	b2World world(b2Vec2(0.0f, 0.0f));
	world.SetContinuousPhysics(true);

	b2PolygonShape sensorShape;
	sensorShape.Set(sensor_left.data(), sensor_left.size());
	b2FixtureDef sensorFixtDef;
	sensorFixtDef.density = 0.0f;
	sensorFixtDef.shape = &sensorShape;
	sensorFixtDef.isSensor = true;
	sensorFixtDef.filter.groupIndex = -1;
	sensorFixtDef.userData = reinterpret_cast<void*>(0);

	b2PolygonShape sensorShapeRight;
	sensorShapeRight.Set(sensor_right.data(), sensor_right.size());
	b2FixtureDef sensorFixtDefRight;
	sensorFixtDefRight.density = 0.0f;
	sensorFixtDefRight.shape = &sensorShapeRight;
	sensorFixtDefRight.isSensor = true;
	sensorFixtDefRight.filter.groupIndex = -1;
	sensorFixtDefRight.userData = reinterpret_cast<void*>(1);

	b2PolygonShape boxBox;
	boxBox.SetAsBox(1.0f, 1.0f);
	b2FixtureDef boxFixtDef;
	boxFixtDef.shape = &boxBox;
	boxFixtDef.density = 1.0f;
	boxFixtDef.filter.groupIndex = -1;

	static constexpr size_t AGENTS = 64;

	struct agent {
		b2Body *body;
		std::array<bool, 2> detected;
		int score;
		int generation_score;
		NEAT::NeuralNetwork nn;
		NEAT::Genome *genome;
		int species;
	};
	std::array<agent, AGENTS> agents;

	for(auto &agent : agents) {
		b2BodyDef boxDef;
		boxDef.type = b2_dynamicBody;
		boxDef.linearDamping = 10.0f;
		boxDef.angularDamping = 10.0f;
		boxDef.position.Set(pos_x_distribution(generator), pos_y_distribution(generator));
		boxDef.linearVelocity = b2Vec2(velocity_distribution(generator), velocity_distribution(generator));
		boxDef.angularVelocity = angular_distribution(generator);
		b2Body *box = world.CreateBody(&boxDef);
		box->CreateFixture(&boxFixtDef);
		box->CreateFixture(&sensorFixtDef);
		box->CreateFixture(&sensorFixtDefRight);
		agent = { box, false, 0 };
	}

	boxBox.SetAsBox(0.5f, 0.5f);
	boxFixtDef.filter.groupIndex = 0;
	boxFixtDef.isSensor = false;

	static constexpr size_t FOODS = 50;
	std::array<b2Body *, FOODS> foods;
	for(auto &food : foods) {
		b2BodyDef foodDef;
		foodDef.type = b2_staticBody;
		foodDef.position.Set(pos_x_distribution(generator), pos_y_distribution(generator));
		food = world.CreateBody(&foodDef);
		food->CreateFixture(&boxFixtDef);
	}

	static bool draw = true;
	glfwSetKeyCallback(window, [] (GLFWwindow*, int key, int, int action, int) {
		if(key == GLFW_KEY_F) {
			draw = action != GLFW_RELEASE;
		}
	});

	NEAT::Parameters params;
	params.PopulationSize = AGENTS;
	params.MinSpecies = 3;
	params.MaxSpecies = 20;
	params.DontUseBiasNeuron = false;
	params.CompatTreshold = 0.1;

	NEAT::Genome s(0, 5, 0, 2, false, NEAT::SIGNED_SIGMOID, NEAT::SIGNED_SIGMOID, 0, params);
	auto &linear_gene = s.m_NeuronGenes[5];
	assert(linear_gene.Type() == NEAT::OUTPUT);
	linear_gene.m_ActFunction = NEAT::UNSIGNED_SIGMOID;
	NEAT::Population pop(s, params, true, 1.0, static_cast<int>(glfwGetTime()));

	[&pop,&agents] {
		size_t n = 0;
		int s = 0;
		for(auto &species : pop.m_Species) {
			for(auto &individual : species.m_Individuals) {
				agents[n].genome = &individual;
				agents[n].species = s;
				individual.BuildPhenotype(agents[n].nn);
				if(++n >= AGENTS) return;
			}
			s++;
		}
	}();

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	double previous_frame = glfwGetTime();
	int generation = 0;
	int step = 0;
	int tick = 0;
	while(true) {
		const int STEPS_PER_GENERATION = 5;
		const int TICKS_PER_STEP = 60 * 15;

		if(tick++ >= TICKS_PER_STEP) {
			tick = 0;
			fprintf(stderr, "Step: %d\n", step);
			step++;
			double total = 0;
			for(auto &agent : agents) {
				total += agent.score;
				agent.generation_score += agent.score;
				agent.score = 0;
			}
			fprintf(stderr, "Average score: %lf\n", total / agents.size());
			if(step >= STEPS_PER_GENERATION) {
				// Generation done
				step = 0;
				fprintf(stderr, "Generation: %d\n", generation);
				for(auto &agent : agents) {
					agent.genome->SetFitness(agent.generation_score / static_cast<double>(STEPS_PER_GENERATION));
					agent.genome->m_Evaluated = true;
					agent.generation_score = 0;
				}
				fprintf(stderr, "Best genome: %lf\n", pop.GetBestGenome().GetFitness());
				pop.Epoch();
				fprintf(stderr, "Best ever  : %lf\n", pop.GetBestFitnessEver());
				fprintf(stderr, "Species: %zu\n", pop.m_Species.size());
				[&pop,&agents] {
					size_t n = 0;
					int s = 0;
					for(auto &species : pop.m_Species) {
						for(auto &individual : species.m_Individuals) {
							agents[n].genome = &individual;
							agents[n].species = s;
							individual.BuildPhenotype(agents[n].nn);
							if(++n >= AGENTS) return;
						}
						s++;
					}
				}();
				generation++;
			}
		}

		world.Step(simulation_timestep, 1, 1);

		// Update agents
		{
			for(auto &agent : agents) {
				auto &body = agent.body;
				const auto angle = body->GetAngle();
				const auto pos = body->GetPosition();
				agent.detected = {};
				for(const b2ContactEdge *edge = body->GetContactList(); edge != nullptr; edge = edge->next) {
					const auto contact = edge->contact;
					const b2Fixture * fixture = contact->GetFixtureA();
					assert(fixture->GetBody() == body);
					if(fixture->IsSensor() && contact->IsTouching()) {
						agent.detected[fixture->GetUserData() != nullptr ? 1 : 0] = true;
					} else if(!fixture->IsSensor() && contact->IsTouching()) {
						auto food = contact->GetFixtureB()->GetBody();
						food->SetTransform(b2Vec2(pos_x_distribution(generator), pos_y_distribution(generator)), 0.0f);
						agent.score++;
					}
				}
				const std::vector<double> inputs {
					agent.detected[0] ? 1.0 : 0.0,
					agent.detected[1] ? 1.0 : 0.0,
					[&body] { auto vel = body->GetLinearVelocity(); return sqrt(vel.x * vel.x + vel.y * vel.y); }(),
					body->GetAngularVelocity(),
					1.0
				};
				agent.nn.Flush();
				agent.nn.Input(const_cast<std::vector<double>&>(inputs));
				agent.nn.Activate();
				const auto output = agent.nn.Output();

				const auto forward = glm::rotate(glm::vec2 { 0.0f, 1.0f }, angle) * static_cast<float>(output[0]) * 1000.0f;
				body->ApplyForceToCenter(b2Vec2 { forward.x, forward.y }, true);
				body->ApplyTorque(output[1] * glm::radians(3.0f * 360.0f * 5.0f), true);

				if(pos.y < -100.0f) body->SetTransform(b2Vec2(pos.x, 100.0f), angle);
				if(pos.y > 100.0f) body->SetTransform(b2Vec2(pos.x, -100.0f), angle);
				if(pos.x < -100.0f * (4.0 / 3.0)) body->SetTransform(b2Vec2(100.0f * (4.0 / 3.0), pos.y), angle);
				if(pos.x > 100.0f * (4.0 / 3.0)) body->SetTransform(b2Vec2(-100.0f * (4.0 / 3.0), pos.y), angle);
			}
		}

		// Update food
		{
			for(auto &food : foods) {
				const auto pos = food->GetPosition();
				const auto angle = food->GetAngle();
				if(pos.y < -100.0f) food->SetTransform(b2Vec2(pos.x, 100.0f), angle);
				if(pos.y > 100.0f) food->SetTransform(b2Vec2(pos.x, -100.0f), angle);
				if(pos.x < -100.0f * (4.0 / 3.0)) food->SetTransform(b2Vec2(100.0f * (4.0 / 3.0), pos.y), angle);
				if(pos.x > 100.0f * (4.0 / 3.0)) food->SetTransform(b2Vec2(-100.0f * (4.0 / 3.0), pos.y), angle);
			}
		}

		glfwPollEvents();
		if(!draw) continue;

		const double this_frame = glfwGetTime();
		const double delta = this_frame - previous_frame;

		if constexpr(trails) {
			glEnable(GL_BLEND);
			black_overlay->activate();
			black_overlay->set_uniform<uniform_type::FLOAT>("alpha", static_cast<float>(delta) * 5.0f);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			glDisable(GL_BLEND);
		} else {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		prog.activate();
		// Draw sensors
		constexpr bool render_sensors = true;
		if constexpr(render_sensors) {
			for(const auto &agent : agents) {
				const auto body = agent.body;
				const b2Vec2 pos = body->GetPosition();
				const float angle = body->GetAngle();
				const glm::mat4 model =
					glm::translate(glm::vec3(pos.x, pos.y, 0.0f)) *
					glm::rotate(angle, glm::vec3(0.0f, 0.0f, 1.0f));
				prog.set_uniform<uniform_type::MAT4>("model", glm::value_ptr(model));

				if(!agent.detected[0]) {
					prog.set_uniform<uniform_type::FLOAT3>("box_colour", 0.0f, 1.0f, 0.0f);
				} else {
					prog.set_uniform<uniform_type::FLOAT3>("box_colour", 1.0f, 0.0f, 0.0f);
				}
				glBindVertexArray(vertex_arrays.sensor);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);

				if(!agent.detected[1]) {
					prog.set_uniform<uniform_type::FLOAT3>("box_colour", 0.0f, 1.0f, 0.0f);
				} else {
					prog.set_uniform<uniform_type::FLOAT3>("box_colour", 1.0f, 0.0f, 0.0f);
				}
				glBindVertexArray(vertex_arrays.sensorr);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
			}
		}

		// Draw agents
		{
			glBindVertexArray(vertex_arrays.rectangle);

			for(const auto &agent : agents) {
				const auto box = agent.body;
				const b2Vec2 pos = box->GetPosition();
				const float angle = box->GetAngle();
				const b2Vec2 vel = box->GetLinearVelocity();
				const glm::mat4 model =
					glm::translate(glm::vec3(pos.x, pos.y, 0.0f)) *
					glm::rotate(angle, glm::vec3(0.0f, 0.0f, 1.0f));
				prog.set_uniform<uniform_type::MAT4>("model", glm::value_ptr(model));
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
				prog.set_uniform<uniform_type::FLOAT3>("box_colour", c.x / 255.0f, c.y / 255.0f, c.z / 255.0f);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}
		}

		// Draw foods
		{
			for(const auto &food : foods) {
				const b2Vec2 pos = food->GetPosition();
				const glm::mat4 model =
					glm::translate(glm::vec3(pos.x, pos.y, 0.0f)) * glm::scale(glm::vec3(0.5f));
				prog.set_uniform<uniform_type::MAT4>("model", glm::value_ptr(model));
				prog.set_uniform<uniform_type::FLOAT3>("box_colour", 0.0f, 0.0f, 1.0f);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}
		}

		glfwSwapBuffers(window);
		previous_frame = this_frame;
	}

	glfwTerminate();
	return 0;
}