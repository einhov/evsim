#ifndef SPECIES_H
#define SPECIES_H

#include <string_view>
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
			normal,
			normal_none,
			shared,
			shared_none,
		};

		static const inline std::unordered_map<std::string_view, training_model_type>
		training_model_by_string {
			{ "normal",      training_model_type::normal      },
			{ "normal_none", training_model_type::normal_none },
			{ "shared",      training_model_type::shared      },
			{ "shared_none", training_model_type::shared_none },
		};
};

}

#endif
