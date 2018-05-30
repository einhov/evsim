#ifndef EVSIM_H
#define EVSIM_H

#include <optional>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <QCoreApplication>

#include "entity.h"

class gui;
class b2World;

namespace evsim {

struct simulation_state {
	unsigned int generation;
	unsigned int step;
	unsigned int tick;

	bool quit;
	bool pause;
	bool draw;
	bool skip;
	bool previous_step;
	std::optional<unsigned int> next_step;

	bool draw_yell;
	bool draw_wall;

	class {
		public:
			inline void move_to(glm::vec2 position, float zoom) {
				centre = position;
				scale = std::max(zoom, min_scale);
				dirty = true;
			}

			inline void pan(glm::vec2 delta) {
				centre += delta / scale;
				dirty = true;
			}

			inline void zoom(float delta) {
				scale = std::max(scale + (delta * scale), min_scale);
				dirty = true;
			}

			inline void aspect_ratio(float a) {
				ratio = a;
				dirty = true;
			}

			glm::mat4 projection() const {
				if(dirty) {
					projection_ =
						glm::ortho(-100.0f * ratio, 100.0f * ratio, -100.0f, 100.0f) *
						glm::scale(glm::vec3 { scale, scale, 1.0 }) *
						glm::translate(glm::vec3 { centre, 0.0 })
					;
					dirty = false;
				}
				return projection_;
			}
		private:
			const float min_scale = 0.1;

			glm::vec2 centre;
			float scale = 1.0;
			float ratio = 4.0 / 3.0;

			mutable bool dirty = true;
			mutable glm::mat4 projection_;
	} camera;

	float simulation_timestep;
	b2World *world;
};

extern std::vector<std::unique_ptr<environmental_entity>> environmental_objects;
extern std::vector<environmental_entity*> to_be_destroyed;
extern simulation_state state;
extern gui *main_gui;

}

#endif
