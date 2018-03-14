#include <random>
#include <iterator>
#include <algorithm>
#include <utility>
#include <optional>
#include <iostream>
#include <boost/filesystem.hpp>

#include <QApplication>

#include <Genome.h>
#include <Parameters.h>
#include <NeuralNetwork.h>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "../../fixture_type.h"
#include "../../body.h"
#include "../../consumable.h"
#include "../../neat_plot.h"
#include "../../config.h"
#include "../../evsim.h"
#include "../../yell.h"
#include "../../lua_conf.h"
#include "../../neat.h"

#include "herbivore_neat.h"
#include "multi_move_herbivore_widget.h"

namespace fs = boost::filesystem;

namespace evsim {
namespace multi_move {

static std::default_random_engine generator(std::random_device{}());
static std::uniform_real_distribution<float> pos_x_distribution(-90.f * (4.0f / 3.0f), -90.f * (4.0f / 3.0f));
static std::uniform_real_distribution<float> pos_y_distribution(-90.0f, 90.0f);

static void relocate_agent(b2Body *body) {
	body->SetTransform(b2Vec2(pos_x_distribution(generator), pos_y_distribution(generator)), pos_y_distribution(generator));
}

void herbivore_neat::clear() {
	for(const auto &agent : agents)
		world.DestroyBody(agent.body);
	agents.clear();
	population.release();
	params.population_size = 0;
	active_genomes = 0;
}

void herbivore_neat::distribute_genomes() {
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

void herbivore_neat::distribute_genomes_shared_fitness(int step) {
	for(auto &agent : agents) {
		agent.genotype = genotypes[step];
		genotypes[step]->BuildPhenotype(agent.phenotype);
	}
}

void herbivore_neat::fill_genome_vector() {
	genotypes.clear();
	for(auto &species : population->m_Species) {
		for(auto &individual : species.m_Individuals) {
			genotypes.emplace_back(&individual);
		}
	}
}

void herbivore_neat::step_shared_fitness(size_t step) {
	int current_score = 0;
	for(auto &agent : agents) {
		current_score += agent.score;
		agent.score = 0;
		agent.body->SetActive(true);
		agent.active = true;
		agent.hear_yell = false;
		agent.body->SetAngularVelocity(0);
		agent.body->SetLinearVelocity(b2Vec2(0,0));
		relocate_agent(agent.body);
	}
	genotypes[step]->SetFitness(current_score / static_cast<double>(agents.size()));
	genotypes[step]->m_Evaluated = true;
	if(step+1 < params.population_size) {
		distribute_genomes_shared_fitness(step+1);
	}
	std::cout << "Shared_fitness_score: " << step << " = " << current_score << std::endl;
}

bool herbivore_neat::initialise(lua_conf &conf, int seed) {
	if(params.population_size > 0)
		clear();

	params.thrust = conf.get_number_default("thrust", 1000.0);
	params.torque = conf.get_number_default("torque", 45.0);
	params.shared_fitness_simulate_count = conf.get_number_default("shared_fitness_simulate_count", 5.0);

	const auto training_model = training_model_by_string.find(
		conf.get_string_default("training_model", "normal")
	);
	if(training_model != training_model_by_string.cend())
		params.training_model = training_model->second;
	else
		throw std::runtime_error("Invalid training_model");

	if(const auto save = conf.get_string("save"); save) {
		params.save_path = fs::path(*save);
		boost::system::error_code ec;
		if(params.save_path && !fs::is_directory(*params.save_path) && !fs::create_directories(*params.save_path, ec))
			params.save_path = std::nullopt;
	}

	if(const auto initial_population = conf.get_string("initial_population"); initial_population) {
		params.initial_population = *initial_population;
		population = load_neat_population(*initial_population);
		params.population_size = population->NumGenomes();
	} else {
		params.population_size = conf.get_integer_default("population_size", 100);
		conf.enter_table_or_empty("neat_params");
		auto neat_params = make_neat_params(conf);
		conf.leave_table();
		neat_params.PopulationSize = params.population_size;

		NEAT::Genome genesis(
			0, 4 + agent::vision_segments * 3, 0, 2, false,
			NEAT::SIGNED_SIGMOID, NEAT::SIGNED_SIGMOID,
			0, neat_params, 0
		);
		population = std::make_unique<NEAT::Population>(genesis, neat_params, true, 1.0, seed);
	}

	agents.resize([this]() -> size_t{
		switch(params.training_model) {
			case training_model_type::normal: [[fallthrough]]
			case training_model_type::normal_none:
				return params.population_size;
			case training_model_type::shared: [[fallthrough]]
			case training_model_type::shared_none:
				return params.shared_fitness_simulate_count;
			default: return 0;
		}
	}());

	for(auto &agent : agents) {
		agent.body = build_body(world);
		agent.body->SetTransform(
			b2Vec2(pos_x_distribution(generator), pos_y_distribution(generator)),
			agent.body->GetAngle()
		);
		agent.body->SetUserData(reinterpret_cast<void*>(&agent));
		agent.species = this;
		agent.active = true;
		agent.body->SetAngularVelocity(0);
		agent.body->SetLinearVelocity(b2Vec2(0,0));
		if(params.training_model == training_model_type::shared)
			agent.internal_species = 0;
	}

	if(params.training_model == training_model_type::shared) {
		fill_genome_vector();
		distribute_genomes_shared_fitness(0);
	} else {
		distribute_genomes();
	}
	return true;
}

void herbivore_neat::pre_tick() {
	for(auto &agent : agents) {
		agent.vision_herbivore = {};
		agent.vision_predator = {};
		agent.vision_wall = {};
	}
}

void herbivore_neat::tick() {
	for(auto &agent : agents) {
		if(!agent.active) continue;
		auto &body = agent.body;
		const auto angle = body->GetAngle();
		const auto pos = body->GetPosition();
		if(pos.x > 100) {
			agent.score +=  100;
			agent.body->SetActive(false);
			agent.active = false;
		}

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
		std::transform(
			agent.vision_wall.cbegin(), agent.vision_wall.cend(),
			std::back_inserter(inputs), vision_inserter
		);
		inputs.emplace_back([&body] { auto vel = body->GetLinearVelocity(); return sqrt(vel.x * vel.x + vel.y * vel.y); }());
		inputs.emplace_back(body->GetAngularVelocity());
		/*
		if(agent.hear_yell) {
			inputs.emplace_back(1.0);
			const auto vec = agent.find_yell_vector();
			inputs.emplace_back(vec.x);
			inputs.emplace_back(vec.y);
		}
		else {
			inputs.emplace_back(0.0);
			inputs.emplace_back(0.0);
			inputs.emplace_back(0.0);
		}
		*/
		inputs.emplace_back(pos.x);
		agent.hear_yell = false;
		inputs.emplace_back(1.0);

		agent.phenotype.Flush();
		agent.phenotype.Input(inputs);
		agent.phenotype.Activate();
		const auto output = agent.phenotype.Output();

		const auto forward =
			glm::rotate(glm::vec2 { 0.0f, 1.0f }, angle) *
			static_cast<float>(output[0]) *
			params.thrust
		;

		body->ApplyForceToCenter(b2Vec2 { forward.x, forward.y }, true);
		body->ApplyTorque(output[1] * params.torque, true);
/*
		if(output[2] >= 0.1) {
			agent.create_yell();
		}
*/
		if(agent.can_yell_timer > 0) {
			agent.can_yell_timer--;
		}
	}
}

glm::vec2 herbivore_neat::agent::find_yell_vector() {
	const auto c = centre_of_yell;
	const auto a = body->GetPosition();
	const auto ca = glm::vec2(c.x, c.y) - glm::vec2(a.x, a.y);
	return glm::rotate(ca, -body->GetAngle());
}

void herbivore_neat::step() {
	double total = 0;
	for(auto &agent : agents) {
		total += agent.score;
		agent.generation_score += agent.score;
		agent.score = 0;
		agent.body->SetActive(true);
		agent.active = true;
		agent.body->SetAngularVelocity(0);
		agent.body->SetLinearVelocity(b2Vec2(0,0));
		relocate_agent(agent.body);

	}
	fprintf(stderr, "NEAT :: Average score: %lf\n", total / agents.size());
}

void herbivore_neat::epoch_shared_fitness() {
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
			*widget, new multi_move_herbivore_widget::epoch_event(
				population->m_Generation,
				total / params.population_size,
				best_score,
				worst_score
			)
		);
	}

	if(params.save_path)
		save();

	fprintf(stderr, "NEAT :: Best genotype: %lf\n", population->GetBestGenome().GetFitness());
	population->Epoch();
	fprintf(stderr, "NEAT :: Best ever    : %lf\n", population->GetBestFitnessEver());
	fprintf(stderr, "NEAT :: Species: %zu\n", population->m_Species.size());
	fill_genome_vector();
	distribute_genomes_shared_fitness(0);
}

void herbivore_neat::epoch(int steps) {
	double total = 0;
	double best_score = std::numeric_limits<double>::min();
	double worst_score = std::numeric_limits<double>::max();
	for(auto &agent : agents) {
		if(agent.generation_score < worst_score)
			worst_score = agent.generation_score;
		if(agent.generation_score > best_score)
			best_score = agent.generation_score;
		total += agent.generation_score / static_cast<double>(steps);
		agent.genotype->SetFitness(agent.generation_score / static_cast<double>(steps));
		agent.genotype->m_Evaluated = true;
		agent.generation_score = 0;
	}
	if(widget) {
		QApplication::postEvent(
			*widget,
			new multi_move_herbivore_widget::epoch_event(
				population->m_Generation,
				total/agents.size(),
				best_score / static_cast<double>(steps),
				worst_score / static_cast<double>(steps)
			)
		);
	}

	if(params.save_path)
		save();

	fprintf(stderr, "NEAT :: Best genotype: %lf\n", population->GetBestGenome().GetFitness());
	population->Epoch();
	fprintf(stderr, "NEAT :: Best ever    : %lf\n", population->GetBestFitnessEver());
	fprintf(stderr, "NEAT :: Species: %zu\n", population->m_Species.size());
	distribute_genomes();
}

void herbivore_neat::epoch_normal_none(int epoch, int steps) {
	double total = 0;
	double best_score = std::numeric_limits<double>::min();
	double worst_score = std::numeric_limits<double>::max();
	for(auto &agent : agents) {
		if(agent.generation_score < worst_score)
			worst_score = agent.generation_score;
		if(agent.generation_score > best_score)
			best_score = agent.generation_score;
		total += agent.generation_score / static_cast<double>(steps);
		agent.generation_score = 0;
	}

	if(widget) {
		QApplication::postEvent(
			*widget,
			new multi_move_herbivore_widget::epoch_event(
				epoch,
				total/agents.size(),
				best_score / static_cast<double>(steps),
				worst_score / static_cast<double>(steps)
			)
		);
	}
}

QWidget *herbivore_neat::make_species_widget() {
	return new multi_move_herbivore_widget(this);
}

void herbivore_neat::save() const {
	if(!params.save_path) return;
	save_neat_population(
		*params.save_path / fs::path(std::to_string(population->m_Generation)),
		*population
	);
}

void herbivore_neat::agent::on_sensor(const msg_contact &contact) {
	using vt = std::array<float, vision_segments>;
	const auto vision_texture = [this,&contact]() -> std::optional<vt*> {
		const auto &foreign_userdata = contact.fixture_foreign->GetUserData();
		const auto &native_fixture_type = *static_cast<fixture_type*>(foreign_userdata);
		switch(native_fixture_type) {
			case fixture_type::torso:
				return &vision_herbivore;
			case fixture_type::torso_predator:
				return &vision_predator;
			case fixture_type::wall:
				return &vision_wall;
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

void herbivore_neat::agent::message(const std::any &msg) {
	if(!active)
		return;
	const auto &type = msg.type();
	if(type == typeid(msg_contact)) {
		const auto &contact = std::any_cast<msg_contact>(msg);
		const auto &native_userdata = contact.fixture_native->GetUserData();
		const auto &native_fixture_type = *static_cast<fixture_type*>(native_userdata);
		const auto &foreign_userdata = contact.fixture_foreign->GetUserData();
		const auto &foreign_fixture_type = *static_cast<fixture_type*>(foreign_userdata);

		if(native_fixture_type == fixture_type::sensor) {
			on_sensor(contact);
		} else if(native_fixture_type == fixture_type::torso && foreign_fixture_type == fixture_type::yell) {
			const yell *yell_heard = static_cast<yell*>(contact.fixture_foreign->GetBody()->GetUserData());
			if(yell_heard->hollerer != this) {
				hear_yell = true;
				centre_of_yell = yell_heard->body->GetPosition();
			}
		}
	} else if(type == typeid(msg_kill)) {
		const auto &consumer = std::any_cast<msg_kill>(msg).consumer;
		score -= 2;
		body->SetActive(false);
		active = false;
		consumer->message(std::make_any<msg_killed>());
	} else if(type == typeid(msg_plot)) {
		plot_genome(*genotype, "selected_agent");
	}
}

void herbivore_neat::agent::create_yell() {
	if(can_yell_timer == 0){
		can_yell_timer = yell_timer_max;
		auto yell_instance = std::make_unique<yell>();
		yell_instance->init_body(species->world, this, body->GetPosition());
		environmental_objects.push_back(std::move(yell_instance));
	}
}

unsigned int herbivore_neat::population_size() const {
	return params.population_size;
}

species::training_model_type herbivore_neat::training_model() const {
	return params.training_model;
}

}
}
