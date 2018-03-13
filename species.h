#ifndef SPECIES_H
#define SPECIES_H

#include <Box2D/Box2D.h>
#include <glm/glm.hpp>
#include <QWidget>

namespace evsim {

class species {
	public:
		virtual ~species() {}
		virtual void pre_tick() = 0;
		virtual void tick() = 0;
		virtual void step() = 0;
		virtual void draw(const glm::mat4 &projection) const = 0;
		virtual QWidget *make_species_widget() {
			return nullptr;
		}

		enum class training_model_type {
			none,
			normal,
			shared
		};
};

}

#endif
