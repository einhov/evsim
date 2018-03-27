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
#include "lua_conf.h"

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

	lua_conf conf(argc >= 2 ? argv[1] : "", argc, argv);
	config::load_config(conf);

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
	state.pause = false;
	state.draw = true;
	state.generation = 0;
	state.step = 0;
	state.tick = 0;
	state.draw_wall = 1;

	if(conf.enter_table("environment", true)) {
		if(const auto draw_wall = conf.get_boolean("draw_wall"); draw_wall)
			state.draw_wall = *draw_wall ? 1 : 0;
		conf.leave_table();
	}

	input_init(window);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glBlendFunc(GL_ONE, GL_ZERO);
	glDisable(GL_DEPTH_TEST);

	auto env = make_environment(conf);
	if(!env) {
		fprintf(stderr, "No environment loaded\n");
		glfwTerminate();
		QApplication::postEvent(main_gui, new gui::quit_event);
		return 1;
	}

	const auto ticks_per_step = env->ticks_per_step();
	const auto steps_per_generation = env->steps_per_generation();

	while(!state.quit) {
		if(!state.pause) {
			if(state.tick == 0)
				env->pre_step();

			// Remove environmental objects flagged for deletion
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

			if(state.tick++ >= ticks_per_step) {
				fprintf(stderr, "Step: %d\n", state.step);
				env->step();

				if(state.skip) {
					state.skip = false;
					if(state.next_step) {
						state.step = state.next_step.value();
						state.next_step.reset();
					} else if(state.previous_step) {
						if(state.step != 0)
							state.step--;
						state.previous_step = false;
					} else {
						state.step++;
					}
				} else {
					state.step++;
				}

				state.tick = 0;

				if(state.step >= steps_per_generation) {
					state.step = 0;
					fprintf(stderr, "Generation: %d\n", state.generation);
					env->epoch();
					state.generation++;
				}
				QApplication::postEvent(main_gui, new gui::step_event);
			}

			env->pre_tick();
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

			env->tick();
		}
		glfwPollEvents();
		if(state.draw && !state.skip) {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			env->draw();
			glfwSwapBuffers(window);
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
