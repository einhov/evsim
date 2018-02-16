#ifndef PREDATOR_NEAT_H
#define PREDATOR_NEAT_H

#include <vector>
#include <memory>

#include <Population.h>
#include <Box2D/Box2D.h>
#include <glm/glm.hpp>

#include "../../entity.h"
#include "../../species.h"

namespace evsim {
namespace multi_food {

class predator_neat : public species {
	public:
		predator_neat(b2World &world) :
		world(world), population_size(0), active_genomes(0) {}
		bool initialise(size_t size, int seed);
		void pre_tick();
		void tick();
		void step();
		void epoch(int steps);
		void draw(const glm::mat4 &projection) const;

	private:
		void clear();
		void distribute_genomes();

		size_t population_size;
		size_t active_genomes;

		class agent : public entity {
			public:
				void message(const std::any &msg) override;
				void on_sensor(const msg_contact &contact);

				b2Body *body;
				int score;
				int generation_score;
				int species;

				static constexpr int vision_segments = 5;
				using vision_texture = std::array<float, vision_segments>;
				vision_texture vision_herbivore;
				vision_texture vision_predator;

				NEAT::Genome *genotype;
				NEAT::NeuralNetwork phenotype;
		};
		std::unique_ptr<NEAT::Population> population;
		std::vector<agent> agents;
		b2World &world;
};

}
}

#endif
