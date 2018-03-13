#include <random>

#include <Genome.h>
#include <Parameters.h>
#include <NeuralNetwork.h>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <math.h>

#include <QApplication>

#include "../../fixture_type.h"
#include "../../body.h"
#include "../../consumable.h"
#include "../../neat_plot.h"
#include "../../config.h"
#include "../../evsim.h"
#include "../../lua_conf.h"
#include "../../neat.h"

#include "herbivore_neat.h"
#include "predator_neat.h"
#include "multi_food_predator_widget.h"

namespace evsim {
namespace multi_food {

static std::default_random_engine generator(std::random_device{}());
static std::uniform_real_distribution<float> pos_x_distribution(-99.0f * (4.0f / 3.0f), 99.0f * (4.0f / 3.0f));
static std::uniform_real_distribution<float> pos_y_distribution(-99.0f, 99.0f);

void predator_neat::clear() {
	for(const auto &agent : agents)
		world.DestroyBody(agent.body);
	agents.clear();
	population.release();
	params.population_size = 0;
	active_genomes = 0;
}

void predator_neat::distribute_genomes() {
	auto &n = active_genomes;
	n = 0;
	int s = 0;
	for(auto &species : population->m_Species) {
		for(auto &individual : species.m_Individuals) {
			agents[n].genotype = &individual;
			agents[n].internal_species = s;
			individual.BuildPhenotype(agents[n].phenotype);
			if(++n >= params.population_size) return;
		}
		s++;
	}
}

void predator_neat::distribute_genomes_shared_fitness(int step) {
	for(auto &agent : agents) {
		agent.genotype = genotypes[step];
		genotypes[step]->BuildPhenotype(agent.phenotype);
	}
}

void predator_neat::fill_genome_vector() {
	genotypes.clear();
	for(auto &species : population->m_Species) {
		for(auto &individual : species.m_Individuals) {
			genotypes.emplace_back(&individual);
		}
	}
}

void predator_neat::step_shared_fitness(size_t step) {
	int current_score = 0;
	for(auto &agent : agents) {
		current_score += agent.score;
		agent.score = 0;
		agent.body->SetActive(true);
		agent.body->SetAngularVelocity(0);
		agent.body->SetLinearVelocity(b2Vec2(0,0));
	}
	genotypes[step]->SetFitness(current_score / static_cast<double>(agents.size()));
	genotypes[step]->m_Evaluated = true;
	if(step+1 < params.population_size) {
		distribute_genomes_shared_fitness(step+1);
	}
	std::cout << "Shared_fitness_score: " << step << " = " << current_score << std::endl;
}

bool predator_neat::initialise(lua_conf &conf, int seed) {
	if(params.population_size > 0)
		clear();

	state.draw_sensors_predator = true;
	params.population_size = conf.get_integer_default("population_size", 100);
	params.thrust = conf.get_number_default("thrust", 1000.0);
	params.torque = conf.get_number_default("torque", 45.0);
	params.eat_delay_max = conf.get_integer_default("eat_delay", 0);
	params.shared_fitness_simulate_count = conf.get_number_default("shared_fitness_simulate_count", 5.0);
	params.consume_opt = [this] {
		if(params.eat_delay_max < 0)
			return consume_options::once;
		else if(params.eat_delay_max > 0)
			return consume_options::delay;
		else
			return consume_options::no_delay;
	}();


	const auto training_model = conf.get_string_default("training_model", "normal");
	if(training_model == "normal") {
		params.training_model = training_model_type::normal;
	} else if(training_model == "shared") {
		params.training_model = training_model_type::shared;
	} else {
		throw std::runtime_error("Invalid training_model");
	}

	conf.enter_table_or_empty("neat_params");
	auto neat_params = make_neat_params(conf);
	conf.leave_table();
	neat_params.PopulationSize = params.population_size;

	if(params.training_model == training_model_type::normal)
		agents.resize(params.population_size);
	else {
		agents.resize(params.shared_fitness_simulate_count);
	}

	for(auto &agent : agents) {
		agent.body = build_predator_body(world);
		agent.body->SetTransform(
			b2Vec2(pos_x_distribution(generator), pos_y_distribution(generator)),
			agent.body->GetAngle()
		);
		agent.body->SetUserData(reinterpret_cast<void*>(&agent));
		agent.eat_delay = 0;
		agent.body->SetAngularVelocity(0);
		agent.body->SetLinearVelocity(b2Vec2(0,0));
		agent.species = this;
		if(params.training_model == training_model_type::shared)
			agent.internal_species = 0;
	}

	NEAT::Genome genesis(
		0, 3 + agent::vision_segments * 2, 0, 2, false,
		NEAT::SIGNED_SIGMOID, NEAT::SIGNED_SIGMOID,
		0, neat_params, 0
	);

	population = std::make_unique<NEAT::Population>(genesis, neat_params, true, 1.0, seed);
	if(params.training_model == training_model_type::shared) {
		fill_genome_vector();
		distribute_genomes_shared_fitness(0);
	} else {
		distribute_genomes();
	}
	return true;
}

void predator_neat::pre_tick() {
	for(auto &agent : agents) {
		agent.vision_herbivore = {};
		agent.vision_predator = {};
	}
}

void predator_neat::tick() {
	for(auto &agent : agents) {
		auto &body = agent.body;

		if(params.consume_opt == consume_options::delay && agent.eat_delay > 0) {
			if(--agent.eat_delay <= 0) {
				body->SetActive(true);
			}
		}

		const auto angle = body->GetAngle();
		const auto pos = body->GetPosition();

		if(pos.y < -100.0f) body->SetTransform(b2Vec2(pos.x, 100.0f), angle);
		if(pos.y > 100.0f) body->SetTransform(b2Vec2(pos.x, -100.0f), angle);
		if(pos.x < -100.0f * (4.0 / 3.0)) body->SetTransform(b2Vec2(100.0f * (4.0 / 3.0), pos.y), angle);
		if(pos.x > 100.0f * (4.0 / 3.0)) body->SetTransform(b2Vec2(-100.0f * (4.0 / 3.0), pos.y), angle);

		static const auto vision_inserter = [](const auto &elem) {
			return elem * 100.0f;
		};

		std::vector<double> inputs;
		std::transform(
			agent.vision_herbivore.cbegin(), agent.vision_herbivore.cend(),
			std::back_inserter(inputs), vision_inserter
		);
		std::transform(
			agent.vision_predator.cbegin(), agent.vision_predator.cend(),
			std::back_inserter(inputs), vision_inserter
		);
		inputs.emplace_back([&body] { auto vel = body->GetLinearVelocity(); return sqrt(vel.x * vel.x + vel.y * vel.y); }());
		inputs.emplace_back(body->GetAngularVelocity());
		inputs.emplace_back(1.0);

		agent.phenotype.Flush();
		agent.phenotype.Input(const_cast<std::vector<double>&>(inputs));
		agent.phenotype.Activate();
		const auto output = agent.phenotype.Output();

		const auto forward =
			glm::rotate(glm::vec2 { 0.0f, 1.0f }, angle) *
			static_cast<float>(output[0]) *
			params.thrust
		;

		body->ApplyForceToCenter(b2Vec2 { forward.x, forward.y }, true);
		body->ApplyTorque(output[1] * params.torque, true);
	}
}

void predator_neat::step() {
	double total = 0;
	for(auto &agent : agents) {
		total += agent.score;
		agent.generation_score += agent.score;
		agent.score = 0;
		agent.eat_delay = 0;
		agent.body->SetActive(true);
		agent.body->SetAngularVelocity(0);
		agent.body->SetLinearVelocity(b2Vec2(0,0));
	}
	fprintf(stderr, "NEAT :: Average score: %lf\n", total / agents.size());
}

QWidget *predator_neat::make_species_widget() {
	return new multi_food_predator_widget(this);
}

void predator_neat::epoch_shared_fitness() {
	double total = 0;
	double best_score = std::numeric_limits<double>::min();
	double worst_score = std::numeric_limits<double>::max();
	for(auto genotype : genotypes) {
		const auto fitness = genotype->GetFitness();
		if(fitness < worst_score)
			worst_score = fitness;
		if(fitness > best_score)
			best_score = fitness;
		total += fitness;
	}
	if(widget) {
		QApplication::postEvent(
			*widget, new multi_food_predator_widget::epoch_event(
				population->m_Generation,
				total / params.population_size,
				best_score,
				worst_score
			)
		);
	}
	fprintf(stderr, "NEAT :: Best genotype: %lf\n", population->GetBestGenome().GetFitness());
	population->Epoch();
	fprintf(stderr, "NEAT :: Best ever    : %lf\n", population->GetBestFitnessEver());
	fprintf(stderr, "NEAT :: Species: %zu\n", population->m_Species.size());
	fill_genome_vector();
	distribute_genomes_shared_fitness(0);
}

void predator_neat::epoch(int steps) {
	double total = 0;
	double killed = 0;
	double best_score = std::numeric_limits<double>::min();
	double worst_score = std::numeric_limits<double>::max();
	for(auto &agent : agents) {
		if(agent.generation_score < worst_score)
			worst_score = agent.generation_score;
		if(agent.generation_score > best_score)
			best_score = agent.generation_score;
		total += agent.generation_score / static_cast<double>(steps);
		killed += agent.generation_score;
		agent.genotype->SetFitness(agent.generation_score / static_cast<double>(steps));
		agent.genotype->m_Evaluated = true;
		agent.generation_score = 0;
	}
	if(widget) {
		QApplication::postEvent(
			*widget, new multi_food_predator_widget::epoch_event(
				population->m_Generation,
				total/agents.size(),
				best_score / static_cast<double>(steps),
				worst_score / static_cast<double>(steps)
			)
		);
	}
	fprintf(stderr, "NEAT :: Best genotype: %lf\n", population->GetBestGenome().GetFitness());
	population->Epoch();
	fprintf(stderr, "NEAT :: Best ever    : %lf\n", population->GetBestFitnessEver());
	fprintf(stderr, "NEAT :: Species: %zu\n", population->m_Species.size());
	distribute_genomes();
}

void predator_neat::agent::on_sensor(const msg_contact &contact) {
	using vt = std::array<float, vision_segments>;
	const auto vision_texture = [this,&contact]() -> std::optional<vt*> {
		const auto &foreign_userdata = contact.fixture_foreign->GetUserData();
		const auto &native_fixture_type = *static_cast<fixture_type*>(foreign_userdata);
		switch(native_fixture_type) {
			case fixture_type::torso:
				return &vision_herbivore;
			case fixture_type::torso_predator:
				return &vision_predator;
			default:
				return {};
		}
	}();

	if(!vision_texture) return;
	auto &vision = **vision_texture;
	const auto forward = glm::rotate(glm::vec2 { 0.0f, 1.0f }, body->GetAngle());
	const glm::vec2 diff = [this,&contact] {
		const auto s = body->GetPosition();
		const auto o = contact.fixture_foreign->GetBody()->GetPosition();
		return glm::vec2(o.x, o.y) - glm::vec2(s.x, s.y);
	}();
	if(diff == glm::vec2(0)) return;
	const auto diff_angle = glm::orientedAngle(forward, glm::normalize(diff));
	const double offset =
		vision_segments -
		(std::clamp(tan(diff_angle) * sensor_length / (0.5f * sensor_width), -1.0f, 1.0f) + 1.0) /
		2.0 * vision_segments
	;
	const auto [integer, fraction] = [offset] {
		float integer;
		float fraction = std::modf(offset, &integer);
		return std::make_pair(static_cast<int>(integer), fraction);
	}();
	static const float hypotenuse = sensor_length / cos(sensor_fov / 2.0);
	const float fac = glm::clamp(hypotenuse - glm::length(diff), 0.0f, hypotenuse) / hypotenuse;
	if(fraction <= 0.5) {
		if(integer > 0)
			vision[integer - 1] += (0.5 - fraction) * fac;
		if(integer < vision_segments)
			vision[integer] += (0.5 + fraction) * fac;
	} else {
		if(integer < vision_segments - 1)
			vision[integer + 1] += (fraction - 0.5) * fac;
		vision[integer] += (1.5 - fraction) * fac;
	}
}

void predator_neat::agent::message(const std::any &msg) {
	const auto &type = msg.type();
	if(type == typeid(msg_contact)) {
		const auto &contact = std::any_cast<msg_contact>(msg);

		const auto &native_userdata = contact.fixture_native->GetUserData();
		const auto &native_fixture_type = *static_cast<fixture_type*>(native_userdata);
		const auto &foreign_userdata = contact.fixture_foreign->GetUserData();
		const auto &foreign_fixture_type = *static_cast<fixture_type*>(foreign_userdata);

		if(native_fixture_type == fixture_type::sensor) {
			on_sensor(contact);
		} else if(native_fixture_type == fixture_type::torso_predator && foreign_fixture_type == fixture_type::torso) {
			const auto &agent = static_cast<entity*>(contact.fixture_foreign->GetBody()->GetUserData());
			agent->message(std::make_any<msg_kill>(msg_kill { this }));
		}
	} else if(type == typeid(msg_killed)) {
		score++;
		if(species->params.consume_opt != consume_options::no_delay) {
			eat_delay = fmax(species->params.eat_delay_max, 1);
			body->SetActive(false);
		}
	} else if(type == typeid(msg_plot)) {
		plot_genome(*genotype, "selected_agent");
	}
}

unsigned int predator_neat::population_size() const {
	return params.population_size;
}

species::training_model_type predator_neat::training_model() const {
	return params.training_model;
}


}
}
