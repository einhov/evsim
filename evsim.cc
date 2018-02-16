#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <Box2D/Box2D.h>
#include <glm/gtc/matrix_transform.hpp>
#include <optional>

#include "evsim.h"
#include "consumable.h"
#include "entity.h"
#include "config.h"
#include "fixture_type.h"
#include "neat_plot.h"
#include "body.h"
#include "yell.h"
#include "input.h"
#include "env/multi_food/environment.h"

namespace evsim {

configuration conf;
std::vector<std::unique_ptr<environmental_entity>> environmental_objects;
std::vector<environmental_entity*> to_be_destroyed;

int evsim(int argc, char **argv) {
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

	conf.projection = glm::ortho(
		-100.0f * (4.0f / 3.0f),
		 100.0f * (4.0f / 3.0f),
		-100.0f,
		 100.0f
	);

	conf.simulation_timestep = 1.0f/60.0f;
	conf.world = new b2World(b2Vec2(0.0f, 0.0f));
	conf.world->SetContinuousPhysics(true);
	conf.draw = true;
	conf.pause = false;
	conf.generation = 0;
	conf.step = 0;
	conf.tick = 0;

	input_init(window);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	double previous_frame = glfwGetTime();
	multi_food::environment env;
	env.init();
	while(true) {
		if(!conf.pause) {
			if(conf.tick++ >= env.TICKS_PER_STEP) {
				conf.tick = 0;
				fprintf(stderr, "Step: %d\n", conf.step);
				conf.step++;
				env.step();
				if(conf.step >= env.STEPS_PER_GENERATION) {
					conf.step = 0;
					fprintf(stderr, "Generation: %d\n", conf.generation);
					env.epoch();
					conf.generation++;
				}
			}

			env.pre_tick();
			conf.world->Step(conf.simulation_timestep, 1, 1);

			// Distribute contact messages
			for(b2Contact *contact = conf.world->GetContactList(); contact != nullptr; contact = contact->GetNext()) {
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

			env.tick();

			for(auto remove : to_be_destroyed) {
				const auto found = std::find_if(
					environmental_objects.begin(), environmental_objects.end(),
					[&remove](const auto &a) { return a.get() == remove; }
				);
				if(found != environmental_objects.end()) {
					environmental_objects.erase(found);
				}
			}
			to_be_destroyed.clear();
		}
		glfwPollEvents();
		if(conf.draw) {
			const double this_frame = glfwGetTime();
			const double delta = this_frame - previous_frame;
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			env.draw();
			glfwSwapBuffers(window);
			previous_frame = this_frame;
		}
	}
	glfwTerminate();
	return 0;
}

}

int main(int argc, char **argv) {
	return evsim::evsim(argc, argv);
}
