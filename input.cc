#ifndef YELL_H
#define YELL_H

#include <array>

#include <Box2D/Box2D.h>
#include <GLFW/glfw3.h>

#include "entity.h"
#include "evsim.h"
#include "fixture_type.h"
#include "neat_plot.h"

#include "env/multi_food/herbivore_neat.h"

namespace evsim {

void input_init(GLFWwindow *window) {
	glfwSetKeyCallback(window, [] (GLFWwindow*, int key, int, int action, int) {
		if(key == GLFW_KEY_F && action == GLFW_PRESS) {
			conf.draw = !conf.draw;
		}
		if(key == GLFW_KEY_O && action == GLFW_PRESS) {
			conf.pause = !conf.pause;
		}
		if(key == GLFW_KEY_S && action == GLFW_PRESS) {
			conf.draw_sensors_herbivore = !conf.draw_sensors_herbivore;
		}
		if(key == GLFW_KEY_D && action == GLFW_PRESS) {
			conf.draw_sensors_predator = !conf.draw_sensors_predator;
		}
		if(key == GLFW_KEY_A && action == GLFW_PRESS) {
			conf.draw_yell = !conf.draw_yell;
		}
	});
	glfwSetMouseButtonCallback(window, [] (GLFWwindow* window, int button, int action, int mods) {
		if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
			double pos_x, pos_y;
			glfwGetCursorPos(window, &pos_x, &pos_y);

			const auto transformed =
				glm::inverse(conf.projection) *
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

			conf.world->QueryAABB(&cb, bounding_box);
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
}

}
#endif
