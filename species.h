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
		virtual void draw(const glm::mat4 &projection) const = 0;
		virtual QWidget *make_species_widget() {
			return nullptr;
		}

		enum class training_model_type {
			normal,
			shared
		};

		static const inline std::unordered_map<std::string_view, training_model_type>
		training_model_by_string {
			{ "normal", training_model_type::normal },
			{ "shared", training_model_type::shared }
		};
};

}

#endif
