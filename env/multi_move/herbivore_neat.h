#ifndef HERBIVORE_NEAT_MULTI_MOVE_H
#define HERBIVORE_NEAT_MULTI_MOVE_H

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

class multi_move_herbivore_widget;

namespace evsim {
namespace multi_move {

struct msg_kill {
	entity *consumer;
};

struct msg_killed {
};

class herbivore_neat : public species {
	friend class ::multi_move_herbivore_widget;
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
		static constexpr bool shared_fitness = false;
		static constexpr size_t shared_fitness_simulate_max = 5;

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
		} params;

		size_t active_genomes;
		std::unique_ptr<NEAT::Population> population;
		std::vector<agent> agents;
		std::vector<NEAT::Genome*> genotypes;
		b2World &world;

		std::optional<multi_move_herbivore_widget*> widget;
		std::atomic_int vision_texture {};
		std::atomic_bool draw_vision {};
};

}
}

#endif
