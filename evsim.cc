#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <Box2D/Box2D.h>
#include <glm/gtc/matrix_transform.hpp>

#include "evsim.h"
#include "consumable.h"
#include "species_neat.h"
#include "predator_neat.h"
#include "entity.h"
#include "config.h"

namespace evsim {

configuration conf;

int evsim(int argc, char **argv) {
	if(!glfwInit()) {
		return -1;
	}

	conf.draw_sensors = true;

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

	const glm::mat4 projection = glm::ortho(
		-100.0f * (4.0f / 3.0f),
		 100.0f * (4.0f / 3.0f),
		-100.0f,
		 100.0f
	);

	const float simulation_timestep = 1.0f/60.0f;

	b2World world(b2Vec2(0.0f, 0.0f));
	world.SetContinuousPhysics(true);

	static species_neat herbivores(world);
	herbivores.initialise(build_config::herbivore_count, static_cast<int>(glfwGetTime()));
	static predator_neat predator(world);
	predator.initialise(build_config::predator_count, static_cast<int>(glfwGetTime()+1));

	std::array<consumable, build_config::food_count> foods;
	for(auto &food : foods)
		food.init_body(world);

	static bool draw = true;
	glfwSetKeyCallback(window, [] (GLFWwindow*, int key, int, int action, int) {
		if(key == GLFW_KEY_F && action == GLFW_PRESS) {
			draw = !draw;
		}
		if(key == GLFW_KEY_P && action == GLFW_PRESS) {
			herbivores.plot = !herbivores.plot ;
		}
		if(key == GLFW_KEY_S && action == GLFW_PRESS) {
			conf.draw_sensors = !conf.draw_sensors;
		}
	});

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
			herbivores.step();
			predator.step();
			if(step >= STEPS_PER_GENERATION) {
				step = 0;
				fprintf(stderr, "Generation: %d\n", generation);			
				herbivores.epoch(STEPS_PER_GENERATION);
				predator.epoch(STEPS_PER_GENERATION);
				generation++;
			}
		}

		world.Step(simulation_timestep, 1, 1);

		herbivores.pre_tick();
		predator.pre_tick();

		// Distribute contact messages
		for(b2Contact *contact = world.GetContactList(); contact != nullptr; contact = contact->GetNext()) {
			if(!contact->IsTouching()) continue;
			auto fixture_A = contact->GetFixtureA();
			auto fixture_B = contact->GetFixtureB();

			if(auto userdata_A = fixture_A->GetBody()->GetUserData(); userdata_A != nullptr)
				static_cast<entity*>(userdata_A)->message(
					std::make_any<msg_contact>(msg_contact { fixture_A, fixture_B })
				);

			if(auto userdata_B = fixture_B->GetBody()->GetUserData(); userdata_B != nullptr)
				static_cast<entity*>(userdata_B)->message(
					std::make_any<msg_contact>(msg_contact { fixture_B, fixture_A })
				);
		}

		// Update agents
		herbivores.tick();
		predator.tick();

		glfwPollEvents();
		if(!draw) continue;

		const double this_frame = glfwGetTime();
		const double delta = this_frame - previous_frame;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		predator.draw(projection);
		herbivores.draw(projection);

		// Draw foods
		for(const auto &food : foods)
			food.draw(projection);

		glfwSwapBuffers(window);
		previous_frame = this_frame;
	}

	glfwTerminate();
	return 0;
}

}

int main(int argc, char **argv) {
	return evsim::evsim(argc, argv);
}
