#include <array>
#include <chrono>
#include <optional>

#include <Box2D/Box2D.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "entity.h"
#include "evsim.h"
#include "fixture_type.h"
#include "neat_plot.h"
#include "ui/gui.h"

namespace evsim {

using std::chrono::steady_clock;
static std::optional<steady_clock::time_point> left_press;
static bool right_press;
static glm::vec2 last_pos;

void input_init(GLFWwindow *window) {
	glfwSetKeyCallback(window, [] (GLFWwindow*, int key, int, int action, int) {
		if(key == GLFW_KEY_F && action == GLFW_PRESS) {
			state.draw = !state.draw;
		}
		if(key == GLFW_KEY_O && action == GLFW_PRESS) {
			state.pause = !state.pause;
		}
		if(key == GLFW_KEY_A && action == GLFW_PRESS) {
			state.draw_yell = !state.draw_yell;
		}
		if(key == GLFW_KEY_Q && action == GLFW_PRESS) {
			state.quit = true;
		}
		if(key == GLFW_KEY_R && action == GLFW_PRESS) {
			state.camera.move_to(glm::vec2 {}, 1.0f);
		}
		if(key == GLFW_KEY_V && action == GLFW_PRESS) {
			static bool sync = true;
			glfwSwapInterval((sync = !sync) ? 1 : 0);
		}
		QCoreApplication::postEvent(main_gui, new gui::refresh_event);
	});

	static constexpr auto pan_callback = [](GLFWwindow *, double x, double y) {
		const auto delta = last_pos - glm::vec2(x, y);
		last_pos = glm::vec2(x, y);
		state.camera.pan(delta * glm::vec2(1.0, -1.0));
	};

	static constexpr auto zoom_callback = [](GLFWwindow *, double x, double y) {
		const auto delta = last_pos - glm::vec2(x, y);
		last_pos = glm::vec2(x, y);
		state.camera.zoom(-delta.x / 100.0);

	};

	glfwSetMouseButtonCallback(window, [] (GLFWwindow* window, int button, int action, int mods) {
		if(button != GLFW_MOUSE_BUTTON_LEFT && button != GLFW_MOUSE_BUTTON_RIGHT) return;

		if(action == GLFW_PRESS) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			last_pos = [window] {
				double x, y;
				glfwGetCursorPos(window, &x, &y);
				return glm::vec2 { x, y };
			}();

			if(button == GLFW_MOUSE_BUTTON_LEFT) {
				left_press = steady_clock::now();
				if(!right_press)
					glfwSetCursorPosCallback(window, pan_callback);
			} else if(button == GLFW_MOUSE_BUTTON_RIGHT) {
				right_press = true;
				glfwSetCursorPosCallback(window, zoom_callback);
			}

		} else if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
			if(!left_press) {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				glfwSetCursorPosCallback(window, nullptr);
			} else {
				glfwSetCursorPosCallback(window, pan_callback);
			}
			right_press = false;

		} else if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
			const bool clicked = left_press && (steady_clock::now() - *left_press < std::chrono::milliseconds(200));
			if(!right_press) {
				glfwSetCursorPosCallback(window, nullptr);
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
			left_press = {};

			if(!clicked) return;

			double pos_x, pos_y;
			glfwGetCursorPos(window, &pos_x, &pos_y);

			const auto transformed =
				glm::inverse(state.camera.projection()) *
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

			state.world->QueryAABB(&cb, bounding_box);
			if(cb.found_fixture) {
				const auto *fixture = *cb.found_fixture;
				auto *agent = static_cast<entity*>(fixture->GetBody()->GetUserData());
				agent->message(std::make_any<msg_plot>());
			}
		}
	});
}

}
