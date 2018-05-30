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
			shared,
			shared_eval
		};

		static inline bool is_sharedish(const training_model_type type) {
			switch(type) {
				case training_model_type::shared: [[fallthrough]]
				case training_model_type::shared_eval:
					return true;
				default:
					return false;
			}
		}

		static const inline std::unordered_map<std::string_view, training_model_type>
		training_model_by_string {
			{ "normal", training_model_type::normal },
			{ "shared", training_model_type::shared },
			{ "shared_eval", training_model_type::shared_eval }
		};
};

}

#endif
