#ifndef PREDATOR_NEAT_MULTI_MOVE_H
#define PREDATOR_NEAT_MULTI_MOVE_H

#include <vector>
#include <memory>

#include <Population.h>
#include <Box2D/Box2D.h>
#include <glm/glm.hpp>
#include <QWidget>

#include "../../entity.h"
#include "../../species.h"
#include "../../lua_conf.h"

class multi_move_predator_widget;

namespace evsim {
namespace multi_move {


class predator_neat : public species {
	friend class ::multi_move_predator_widget;
	public:

		enum class consume_options {
			once,
			delay,
			no_delay
		};

		predator_neat(b2World &world) :
			params{}, active_genomes(0), world(world) {}
		bool initialise(lua_conf &conf, int seed);
		void pre_tick();
		void tick();
		void step();
		void step_shared_fitness(size_t epoch_step);
		void epoch(int steps);
		void epoch_shared_fitness();
		void draw(const glm::mat4 &projection) const;
		QWidget *make_species_widget() override;
		unsigned int population_size() const;
		static constexpr bool shared_fitness = false;
		static constexpr size_t shared_fitness_simulate_max = 5;

	private:
		class agent : public entity {
			public:
				void message(const std::any &msg) override;
				void on_sensor(const msg_contact &contact);

				int score;
				int generation_score;
				int internal_species;
				int eat_delay;
				b2Body *body;
				predator_neat *species;

				static constexpr int vision_segments = 3;
				using vision_texture = std::array<float, vision_segments>;
				vision_texture vision_herbivore;
				vision_texture vision_predator;
				vision_texture vision_wall;

				NEAT::Genome *genotype;
				NEAT::NeuralNetwork phenotype;
		};

		void clear();
		void distribute_genomes();
		void fill_genome_vector();
		void distribute_genomes_shared_fitness(int step);

		struct {
			size_t population_size;
			float thrust;
			float torque;
			consume_options consume_opt = consume_options::no_delay;
			int eat_delay_max = 0;
		} params;

		size_t active_genomes;
		std::unique_ptr<NEAT::Population> population;
		std::vector<agent> agents;
		std::vector<NEAT::Genome*> genotypes;
		b2World &world;

		std::optional<multi_move_predator_widget*> widget;
		std::atomic_int vision_texture {};
		std::atomic_bool draw_vision {};
};

}
}

#endif
