#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <Box2D/Box2D.h>
#include <glm/gtc/matrix_transform.hpp>
#include <optional>

#include "evsim.h"
#include "consumable.h"
#include "species_neat.h"
#include "predator_neat.h"
#include "entity.h"
#include "config.h"
#include "fixture_type.h"
#include "neat_plot.h"
#include "body.h"
#include "yell.h"

namespace evsim {

configuration conf;
std::vector<std::unique_ptr<environmental_entity>> environmental_objects;
std::vector<environmental_entity*> to_be_destroyed;

 class ClickQueryCallback : public b2QueryCallback {
 public:
	std::optional<const b2Fixture*> found_fixture;

	bool ReportFixture(b2Fixture* fixture) {
		 switch(*static_cast<fixture_type*>(fixture->GetUserData())) {
			case fixture_type::torso:
				found_fixture = fixture;
				return false;
			case fixture_type::torso_predator:
				found_fixture = fixture;
				return false;
			case fixture_type::yell:
				std::cout << "Found yell" << std::endl;
				return false;
			default:
				return true;
		}
	}
 };

int evsim(int argc, char **argv) {
	if(!glfwInit()) {
		return -1;
	}

	conf.draw_sensors_herbivore = true;
	conf.draw_sensors_predator = true;

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

	for(int i = 0; i < build_config::food_count; i++) {
		auto consumable_instance = std::make_unique<consumable>();
		consumable_instance->init_body(world);
		environmental_objects.emplace_back(std::move(consumable_instance));
	}

	static bool draw = true;
	static bool pause = false;
	glfwSetKeyCallback(window, [] (GLFWwindow*, int key, int, int action, int) {
		if(key == GLFW_KEY_F && action == GLFW_PRESS) {
			draw = !draw;
		}
		if(key == GLFW_KEY_O && action == GLFW_PRESS) {
			pause = !pause;
		}
		if(key == GLFW_KEY_P && action == GLFW_PRESS) {
			herbivores.plot = !herbivores.plot ;
		}
		if(key == GLFW_KEY_S && action == GLFW_PRESS) {
			conf.draw_sensors_herbivore = !conf.draw_sensors_herbivore;
		}
		if(key == GLFW_KEY_D && action == GLFW_PRESS) {
			conf.draw_sensors_predator = !conf.draw_sensors_predator;
		}
	});
	glfwSetMouseButtonCallback(window, [] (GLFWwindow* window, int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
			double pos_x, pos_y;
			glfwGetCursorPos(window, &pos_x, &pos_y);
			glm::vec4 transformed {
				-1 + 2.0*(pos_x)/(1024),
				1 - 2.0*(pos_y)/(768),
				0,
				1
			};
			transformed = glm::inverse(projection) * transformed;
			ClickQueryCallback queryCallback;
			b2Vec2 lower(transformed.x-0.01, transformed.y-0.01);
			b2Vec2 upper(transformed.x+0.01, transformed.y+0.01);
			b2AABB aabb;
			aabb.lowerBound = lower;
			aabb.upperBound = upper;
			world.QueryAABB( &queryCallback, aabb);
			if(queryCallback.found_fixture) {
				const auto *fixture = *queryCallback.found_fixture;
				auto *agent = static_cast<entity*>(fixture->GetBody()->GetUserData());
				agent->message(std::make_any<msg_plot>(msg_plot {}));
				const b2Vec2 pos = fixture->GetBody()->GetPosition();
				printf("Mouse pos : %f, %f : \n", transformed.x,transformed.y);
				printf("Object found at : %f, %f\n", pos.x, pos.y);
				fflush(stdout);
			}
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
		if(!pause){
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

			// Update objects
			herbivores.tick();
			predator.tick();
			for(auto &env_obj : environmental_objects) {
				env_obj->tick();
			}

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
		if(!draw) continue;

		const double this_frame = glfwGetTime();
		const double delta = this_frame - previous_frame;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Draw environmental objects
		for(const auto &env_obj : environmental_objects){
			env_obj->draw(projection);
		}

		predator.draw(projection);
		herbivores.draw(projection);

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
