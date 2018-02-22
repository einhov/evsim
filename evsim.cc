#include <thread>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <Box2D/Box2D.h>
#include <glm/gtc/matrix_transform.hpp>
#include <optional>

#include <QApplication>
#include <QPushButton>
#include "ui/gui.h"

#include "evsim.h"
#include "consumable.h"
#include "entity.h"
#include "species.h"
#include "config.h"
#include "fixture_type.h"
#include "neat_plot.h"
#include "body.h"
#include "yell.h"
#include "input.h"

#include "env/environments.h"

namespace evsim {

gui *main_gui;
simulation_state state;
std::vector<std::unique_ptr<environmental_entity>> environmental_objects;
std::vector<environmental_entity*> to_be_destroyed;

int evsim(int argc, char **argv) {
	if(!glfwInit()) {
		return -1;
	}

	state.draw = true;

	QCoreApplication::postEvent(main_gui, new gui::refresh_event);

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *window = glfwCreateWindow(1024, 768, "evsim", nullptr, nullptr);
	if(!window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glewExperimental = true;

	if(glewInit() != GLEW_OK) {
		return -1;
	}

	state.projection = glm::ortho(
		-100.0f * (4.0f / 3.0f),
		 100.0f * (4.0f / 3.0f),
		-100.0f,
		 100.0f
	);

	state.simulation_timestep = 1.0f/60.0f;
	state.world = new b2World(b2Vec2(0.0f, 0.0f));
	state.world->SetContinuousPhysics(true);
	state.draw = true;
	state.pause = false;
	state.generation = 0;
	state.step = 0;
	state.tick = 0;

	input_init(window);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glBlendFunc(GL_ONE, GL_ZERO);
	glEnable(GL_DEPTH_TEST);
	double previous_frame = glfwGetTime();
	multi_food::environment env;
	env.init();
	while(!state.quit) {
		if(!state.pause) {
			if(state.tick++ >= env.TICKS_PER_STEP) {
				state.tick = 0;
				fprintf(stderr, "Step: %d\n", state.step);
				state.step++;
				env.step();
				if(state.step >= env.STEPS_PER_GENERATION) {
					state.step = 0;
					fprintf(stderr, "Generation: %d\n", state.generation);
					env.epoch();
					state.generation++;
				}
				QApplication::postEvent(main_gui, new gui::step_event);
			}

			env.pre_tick();
			state.world->Step(state.simulation_timestep, 1, 1);

			// Distribute contact messages
			for(b2Contact *contact = state.world->GetContactList(); contact != nullptr; contact = contact->GetNext()) {
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
		if(state.draw) {
			const double this_frame = glfwGetTime();
			const double delta = this_frame - previous_frame;
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			env.draw();
			glfwSwapBuffers(window);
			previous_frame = this_frame;
		}
	}
	glfwTerminate();
	QApplication::postEvent(main_gui, new gui::quit_event);
	return 0;
}

}

int main(int argc, char **argv) {
	QApplication app(argc, argv);
	evsim::main_gui = new gui();
	evsim::main_gui->show();
	std::thread simulation_thread(evsim::evsim, argc, argv);
	app.exec();
	simulation_thread.join();
}
