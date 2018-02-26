#ifndef HERBIVORE_NEAT_MULTI_MOVE_H
#define HERBIVORE_NEAT_MULTI_MOVE_H

#include <vector>
#include <memory>

#include <Population.h>
#include <Box2D/Box2D.h>
#include <glm/glm.hpp>

#include "../../entity.h"
#include "../../species.h"

namespace evsim {
namespace multi_move {

struct msg_kill {
	entity *consumer;
};

struct msg_killed {
};
static void relocate_agent(b2Body *body);

class herbivore_neat : public species {
	public:
		herbivore_neat(b2World &world) :
		        world(world), population_size(0), active_genomes(0) {}
		bool initialise(size_t size, int seed);
		void pre_tick();
		void tick();
		void step();
		void epoch(int steps);
		void draw(const glm::mat4 &projection) const;

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
				bool active;

				static constexpr int vision_segments = 3;
				using vision_texture = std::array<float, vision_segments>;
				vision_texture vision_food;
				vision_texture vision_herbivore;
				vision_texture vision_predator;
				vision_texture vision_wall;

				NEAT::Genome *genotype;
				NEAT::NeuralNetwork phenotype;
		};

		void clear();
		void distribute_genomes();

		size_t population_size;
		size_t active_genomes;

		std::unique_ptr<NEAT::Population> population;
		std::vector<agent> agents;
		b2World &world;
};

}
}

#endif
