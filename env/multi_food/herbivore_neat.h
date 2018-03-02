#ifndef HERBIVORE_NEAT_MULTI_FOOD_H
#define HERBIVORE_NEAT_MULTI_FOOD_H

#include <vector>
#include <memory>

#include <Population.h>
#include <Box2D/Box2D.h>
#include <glm/glm.hpp>
#include <QWidget>

#include "../../entity.h"
#include "../../species.h"

class multi_food_herbivore_widget;

namespace evsim {
namespace multi_food {

struct msg_kill {
	entity *consumer;
};

struct msg_killed {
};

class herbivore_neat : public species {
	friend class ::multi_food_herbivore_widget;
	public:
		herbivore_neat(b2World &world) :
		        population_size(0), active_genomes(0), world(world) {}
		bool initialise(size_t size, int seed);
		void pre_tick();
		void tick();
		void step();
		void epoch(int steps);
		void draw(const glm::mat4 &projection) const;
		QWidget *make_species_widget();

	private:
		friend class agent;
		class agent : public entity {
			public:
				void message(const std::any &msg) override;
				void on_sensor(const msg_contact &contact);
				void create_yell();
				glm::vec2 find_yell_vector();

				b2Vec2 centre_of_yell;
				int yell_timer_max = 60;
				int can_yell_timer = yell_timer_max;
				bool hear_yell = false;
				int score;
				int generation_score;
				int internal_species;
				herbivore_neat* species;
				b2Body *body;

				static constexpr int vision_segments = 3;
				using vision_texture = std::array<float, vision_segments>;
				vision_texture vision_food;
				vision_texture vision_herbivore;
				vision_texture vision_predator;

				NEAT::Genome *genotype;
				NEAT::NeuralNetwork phenotype;
		};

		void clear();
		void distribute_genomes();

		size_t population_size;
		size_t active_genomes;
		std::optional<multi_food_herbivore_widget*> widget;
		std::unique_ptr<NEAT::Population> population;
		std::vector<agent> agents;
		b2World &world;

		std::atomic_int vision_texture {};
		std::atomic_bool draw_vision {};
};

}
}

#endif
