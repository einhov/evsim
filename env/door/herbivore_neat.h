#ifndef HERBIVORE_NEAT_DOOR_H
#define HERBIVORE_NEAT_DOOR_H

#include <vector>
#include <memory>
#include <optional>
#include <boost/filesystem.hpp>

#include <Population.h>
#include <Box2D/Box2D.h>
#include <glm/glm.hpp>
#include <QWidget>

#include "../../entity.h"
#include "../../species.h"
#include "../../lua_conf.h"

class door_herbivore_widget;

namespace evsim {
namespace door {

class environment;

struct msg_kill {
	entity *consumer;
};

struct msg_killed {
};

class herbivore_neat : public species {
	friend class ::door_herbivore_widget;
	public:
		herbivore_neat(b2World &world, environment &env) :
			params{}, active_genomes(0), world(world), env(env)
			{}
		bool initialise(lua_conf &conf, int seed);
		void pre_tick();
		void tick();
		void pre_step();
		void step_normal();
		void step_shared(size_t epoch_step);
		void epoch_normal(int epoch, int steps);
		void epoch_shared(int epoch);
		void draw(const glm::mat4 &projection) const;
		QWidget *make_species_widget();
		unsigned int population_size() const;
		training_model_type training_model() const;
		void save(double avg, double high, double low, double agents) const;

		inline bool train() const { return params.train; }

	private:
		friend class agent;
		class agent : public entity {
			public:
				void message(const std::any &msg) override;
				void on_sensor(const msg_contact &contact);
				void create_yell();
				glm::vec2 find_yell_vector();

				int score;
				int generation_score;
				int internal_species;
				herbivore_neat* species;
				b2Body *body;
				bool active;

				bool draw_vision;
				static constexpr int vision_segments = 3;
				using vision_texture = std::array<float, vision_segments>;
				vision_texture vision_herbivore;
				vision_texture vision_predator;
				vision_texture vision_wall;
				vision_texture vision_goal;
				vision_texture vision_button;

				bool is_on_button = false;
				bool yell_detected = false;
				b2Vec2 yell_vector;
				int yell_cooldown = 0;

				NEAT::Genome *genotype;
				NEAT::NeuralNetwork phenotype;
		};

		void clear();
		void distribute_genomes();
		void fill_genome_vector();
		void distribute_genomes_shared(int step);

		struct {
			size_t population_size;
			size_t shared_fitness_simulate_count;
			training_model_type training_model;
			bool train;
			float thrust;
			float torque;
			int yell_delay;
			std::string initial_population;
			std::optional<boost::filesystem::path> save_path;
			size_t avg_window;
		} params;

		size_t active_genomes;
		size_t tick_goal_count;
		std::unique_ptr<NEAT::Population> population;
		std::vector<agent> agents;
		std::vector<NEAT::Genome*> genotypes;
		b2World &world;
		environment &env;
		std::vector<agent*> agents_on_buttons;
		std::optional<door_herbivore_widget*> widget;
		std::atomic_int vision_texture {};
		std::atomic_bool draw_vision {};
		size_t agents_in_goal;
};

}
}

#endif
