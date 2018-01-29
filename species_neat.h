#ifndef SPECIES_NEAT_H
#define SPECIES_NEAT_H

#include <vector>
#include <memory>

#include <Population.h>
#include <Box2D/Box2D.h>
#include <glm/glm.hpp>

#include "entity.h"

namespace evsim {

class species_neat {
	public:
		species_neat(b2World &world) :
		        world(world), population_size(0), active_genomes(0), plot(false) {}
		bool initialise(size_t size, int seed);
		void pre_tick();
		void tick();
		void step();
		void epoch(int steps);
		void draw(const glm::mat4 &projection) const;
                void plot_best();
                bool plot;

	private:
		void clear();
		void distribute_genomes();

		size_t population_size;
		size_t active_genomes;

		class agent : public entity {
			public:
				static constexpr int vision_segments = 3;
				void message(const std::any &msg) override;
				void on_sensor(const msg_contact &contact);

				b2Body *body;
				int score;
				int generation_score;
				int species;
				std::array<float, vision_segments> vision;

				NEAT::Genome *genotype;
				NEAT::NeuralNetwork phenotype;
		};
		std::unique_ptr<NEAT::Population> population;
		std::vector<agent> agents;
		b2World &world;
};

};

#endif
