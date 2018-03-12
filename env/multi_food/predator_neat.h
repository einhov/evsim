#ifndef PREDATOR_NEAT_MULTI_FOOD_H
#define PREDATOR_NEAT_MULTI_FOOD_H

#include <vector>
#include <memory>
#include <atomic>

#include <Population.h>
#include <Box2D/Box2D.h>
#include <glm/glm.hpp>
#include <QWidget>

#include "../../entity.h"
#include "../../species.h"

class multi_food_predator_widget;

namespace evsim {
namespace multi_food {

class predator_neat : public species {
	friend class ::multi_food_predator_widget;
	public:

		enum class consume_options {
			once,
			delay,
			no_delay
		};

		predator_neat(b2World &world) :
			population_size(0), active_genomes(0), world(world) {}
		bool initialise(size_t size, int seed);
		void pre_tick();
		void tick();
		void step();
		void step_shared_fitness(size_t epoch_step);
		void epoch(int steps);
		void epoch_shared_fitness();
		void draw(const glm::mat4 &projection) const;
		QWidget *make_species_widget() override;
		static constexpr consume_options consume_opt = consume_options::delay;
		static const int eat_delay_max = 60;
		static constexpr bool shared_fitness = false;
		static constexpr size_t shared_fitness_simulate_max = 5;

	private:
		void clear();
		void distribute_genomes();
		void fill_genome_vector();
		void distribute_genomes_shared_fitness(int step);

		size_t population_size;
		size_t active_genomes;

		class agent : public entity {
			public:
				void message(const std::any &msg) override;
				void on_sensor(const msg_contact &contact);

				int eat_delay;
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
		std::optional<multi_food_predator_widget*> widget;
		std::unique_ptr<NEAT::Population> population;
		std::vector<agent> agents;
		std::vector<NEAT::Genome*> genotypes;
		b2World &world;
		std::atomic_int vision_texture {};
		std::atomic_bool draw_vision {};
};

}
}

#endif
