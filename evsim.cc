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
#include "species_neat.h"
#include "predator_neat.h"
#include "entity.h"
#include "species.h"
#include "config.h"
#include "fixture_type.h"
#include "neat_plot.h"
#include "body.h"

namespace evsim {

gui *main_gui;

simulation_state state;

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

	static const glm::mat4 projection = glm::ortho(
		-100.0f * (4.0f / 3.0f),
		 100.0f * (4.0f / 3.0f),
		-100.0f,
		 100.0f
	);

	const float simulation_timestep = 1.0f/60.0f;

	static b2World world(b2Vec2(0.0f, 0.0f));
	world.SetContinuousPhysics(true);

	static species_neat herbivores(world);
	herbivores.initialise(build_config::herbivore_count, static_cast<int>(glfwGetTime()));
	static predator_neat predator(world);
	predator.initialise(build_config::predator_count, static_cast<int>(glfwGetTime()+1));

	QApplication::postEvent(main_gui, new gui::add_species_event(&herbivores));
	QApplication::postEvent(main_gui, new gui::add_species_event(&predator));

	std::array<consumable, build_config::food_count> foods;
	for(auto &food : foods)
		food.init_body(world);

	glfwSetKeyCallback(window, [] (GLFWwindow*, int key, int, int action, int) {
		std::scoped_lock<std::mutex> lock(evsim::state.mutex);
		if(key == GLFW_KEY_F && action == GLFW_PRESS) {
			state.draw = !state.draw;
		}
		if(key == GLFW_KEY_O && action == GLFW_PRESS) {
			state.pause = !state.pause;
		}
		if(key == GLFW_KEY_P && action == GLFW_PRESS) {
			herbivores.plot = !herbivores.plot ;
		}
		if(key == GLFW_KEY_Q && action == GLFW_PRESS) {
			state.quit = true;
		}
		QCoreApplication::postEvent(main_gui, new gui::refresh_event);
	});

	glfwSetMouseButtonCallback(window, [] (GLFWwindow* window, int button, int action, int mods) {
		if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
			double pos_x, pos_y;
			glfwGetCursorPos(window, &pos_x, &pos_y);

			const auto transformed =
				glm::inverse(projection) *
				glm::vec4 { -1 + 2.0*(pos_x)/(1024), 1 - 2.0*(pos_y)/(768), 0, 1 }
			;

			struct : public b2QueryCallback {
				std::optional<const b2Fixture*> found_fixture;
				bool ReportFixture(b2Fixture* fixture) {
					switch(*static_cast<fixture_type*>(fixture->GetUserData())) {
						case fixture_type::torso:
							found_fixture = fixture;
							return false;
						case fixture_type::torso_predator:
							found_fixture = fixture;
							return false;
						default:
							return true;
					}
				}
			} cb;

			const b2AABB bounding_box {
				b2Vec2 { transformed.x - 0.01f, transformed.y - 0.01f },
				b2Vec2 { transformed.x + 0.01f, transformed.y + 0.01f }
			};

			world.QueryAABB(&cb, bounding_box);
			if(cb.found_fixture) {
				const auto *fixture = *cb.found_fixture;
				auto *agent = static_cast<entity*>(fixture->GetBody()->GetUserData());
				agent->message(std::make_any<msg_plot>());
				const b2Vec2 pos = fixture->GetBody()->GetPosition();
				printf("Mouse pos : %f, %f : \n", transformed.x,transformed.y);
				printf("Object found at : %f, %f\n", pos.x, pos.y);
				fflush(stdout);
			}
		}
	});

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glBlendFunc(GL_ONE, GL_ZERO);
	glEnable(GL_DEPTH_TEST);
	double previous_frame = glfwGetTime();
	while(!state.quit) {
		if(!state.pause){
			const int STEPS_PER_GENERATION = 15;
			const int TICKS_PER_STEP = 60 * 15;

			if(state.tick++ >= TICKS_PER_STEP) {
				state.tick = 0;
				fprintf(stderr, "Step: %d\n", state.step);
				state.step++;
				herbivores.step();
				predator.step();
				if(state.step >= STEPS_PER_GENERATION) {
					state.step = 0;
					fprintf(stderr, "Generation: %d\n", state.generation);
					herbivores.epoch(STEPS_PER_GENERATION);
					predator.epoch(STEPS_PER_GENERATION);
					state.generation++;
				}
				QApplication::postEvent(main_gui, new gui::step_event);
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
		}

		glfwPollEvents();
		if(!state.draw) continue;

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
