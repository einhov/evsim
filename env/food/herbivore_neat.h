#ifndef HERBIVORE_NEAT_FOOD_H
#define HERBIVORE_NEAT_FOOD_H

#include <vector>
#include <memory>
#include <optional>
#include <atomic>
#include <Genome.h>

#include <Population.h>
#include <Box2D/Box2D.h>
#include <glm/glm.hpp>
#include <QWidget>

#include "../../entity.h"
#include "../../species.h"
#include "../../lua_conf.h"

class food_herbivore_widget;

namespace evsim {
namespace food {

struct msg_kill {
	entity *consumer;
};

struct msg_killed {
};

class herbivore_neat : public species {
	friend class ::food_herbivore_widget;
	public:
		herbivore_neat(b2World &world) :
			params{}, active_genomes(0), world(world) {}
		bool initialise(lua_conf &conf, int seed);
		void pre_tick();
		void tick();
		void step();
		void step_shared_fitness(size_t epoch_step);
		void epoch(int steps);
		void epoch_shared_fitness();
		void draw(const glm::mat4 &projection) const;
		QWidget *make_species_widget();
		unsigned int population_size() const;
		training_model_type training_model() const;

	private:
		friend class agent;
		class agent : public entity {
			public:
				void message(const std::any &msg) override;
				void on_sensor(const msg_contact &contact);

				b2Body *body;
				int score;
				int generation_score;
				int internal_species;
				herbivore_neat* species;

				static constexpr int vision_segments = 5;
				using vision_texture = std::array<float, vision_segments>;
				vision_texture vision_food;
				vision_texture vision_herbivore;
				vision_texture vision_predator;

				NEAT::Genome *genotype;
				NEAT::NeuralNetwork phenotype;
		};

		void clear();
		void distribute_genomes();
		void fill_genome_vector();
		void distribute_genomes_shared_fitness(int step);

		struct {
			size_t population_size;
			size_t shared_fitness_simulate_count;
			training_model_type training_model;
			float thrust;
			float torque;
		} params;

		size_t active_genomes;
		std::unique_ptr<NEAT::Population> population;
		std::vector<agent> agents;
		std::vector<NEAT::Genome*> genotypes;
		b2World &world;

		std::optional<food_herbivore_widget*> widget;
		std::atomic_int vision_texture {};
		std::atomic_bool draw_vision {};
};

}
}

#endif
