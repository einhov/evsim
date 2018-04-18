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
#include "../../lua_conf.h"
#include "../../neat.h"

#include "herbivore_neat.h"
#include "door_herbivore_widget.h"
#include "environment.h"

namespace fs = boost::filesystem;

namespace evsim {
namespace door {

static std::default_random_engine generator(std::random_device{}());
static std::uniform_real_distribution<float> pos_x_distribution(-90.f * (4.0f / 3.0f), -90.f * (4.0f / 3.0f));
static std::uniform_real_distribution<float> pos_y_distribution(-90.0f, 90.0f);
static std::uniform_real_distribution<float> rotation_distribution(0.0f, glm::radians(360.0f));

static void relocate_agent(b2Body *body) {
	body->SetTransform(
		b2Vec2(pos_x_distribution(generator), pos_y_distribution(generator)),
		glm::radians(-90.0)
	);
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

void herbivore_neat::distribute_genomes_shared(int step) {
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

bool herbivore_neat::initialise(lua_conf &conf, int seed) {
	if(params.population_size > 0)
		clear();

	params.thrust = conf.get_number_default("thrust", 1000.0);
	params.torque = conf.get_number_default("torque", 45.0);
	params.avg_window = conf.get_integer_default("avg_window", 21);
	params.shared_fitness_simulate_count = conf.get_number_default("shared_fitness_simulate_count", 5.0);

	const auto training_model = training_model_by_string.find(
		conf.get_string_default("training_model", "normal")
	);
	if(training_model != training_model_by_string.cend())
		params.training_model = training_model->second;
	else
		throw std::runtime_error("Invalid training_model");

	params.train = conf.get_boolean_default("train", true);

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

		//5: 1 angular vel, 1 linear vel, 1 direction, 1 on_button, 1 bias (1 x pos, 1 y pos removed for now)
		//vision 4: = 1 goal, 1 button, 1 herbivore, 1 wall
		NEAT::Genome genesis(
			0, 5 + agent::vision_segments * 4, 0, 2, false,
			NEAT::SIGNED_SIGMOID, NEAT::SIGNED_SIGMOID,
			0, neat_params, 0
		);
		population = std::make_unique<NEAT::Population>(genesis, neat_params, true, 1.0, seed);
	}

	agents.resize([this]() -> size_t{
		switch(params.training_model) {
			case training_model_type::normal:
				return params.population_size;
			case training_model_type::shared:
				return params.shared_fitness_simulate_count;
			default: return 0;
		}
	}());

	for(auto &agent : agents) {
		agent.body = build_body(world);
		relocate_agent(agent.body);
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
		distribute_genomes_shared(0);
	} else {
		distribute_genomes();
	}
	agents_in_goal = 0;
	return true;
}

void herbivore_neat::pre_tick() {
	for(auto &agent : agents) {
		agent.vision_herbivore = {};
		agent.vision_predator = {};
		agent.vision_wall = {};
		agent.vision_goal = {};
		agent.vision_button = {};
		agent.is_on_button = false;
		agents_on_buttons.clear();
		tick_goal_count = 0;
	}
}

void herbivore_neat::tick() {
	if(tick_goal_count > 0) {
		for(auto agent : agents_on_buttons) {
			agent->score += (100.0 * tick_goal_count) / agents_on_buttons.size();
		}
	}

	for(auto &agent : agents) {
		if(!agent.active) continue;
		auto &body = agent.body;
		const auto angle = body->GetAngle();

		static const auto vision_inserter = [](const auto &elem) {
			return elem * 100.0f;
		};

		std::vector<double> inputs;

		std::transform(
			agent.vision_herbivore.cbegin(), agent.vision_herbivore.cend(),
			std::back_inserter(inputs), vision_inserter
		);

		std::transform(
			agent.vision_wall.cbegin(), agent.vision_wall.cend(),
			std::back_inserter(inputs), vision_inserter
		);

		std::transform(
			agent.vision_goal.cbegin(), agent.vision_goal.cend(),
			std::back_inserter(inputs), vision_inserter
		);

		std::transform(
			agent.vision_button.cbegin(), agent.vision_button.cend(),
			std::back_inserter(inputs), vision_inserter
		);

		inputs.emplace_back([&body] { auto vel = body->GetLinearVelocity(); return sqrt(vel.x * vel.x + vel.y * vel.y); }());
		inputs.emplace_back(body->GetAngularVelocity());
		inputs.emplace_back(agent.body->GetAngle());

		inputs.emplace_back(agent.is_on_button);

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
	}
}

void herbivore_neat::pre_step() {
	for(auto &agent : agents) {
		agent.body->SetActive(true);
		agent.active = true;
		agent.body->SetAngularVelocity(0);
		agent.body->SetLinearVelocity(b2Vec2(0,0));
		relocate_agent(agent.body);
		agent.score = 0;
	}
}

void herbivore_neat::step_normal() {
	double total = 0;
	for(auto &agent : agents) {
		total += agent.score;
		agent.generation_score += agent.score;
		agent.score = 0;
	}
}

void herbivore_neat::step_shared(size_t step) {
	int current_score = 0;
	for(auto &agent : agents) {
		current_score += agent.score;
		agent.score = 0;
	}
	const double fitness = agents_in_goal;
	genotypes[step]->SetFitness(fitness);
	genotypes[step]->m_Evaluated = true;

	if(params.train && params.save_path) {
		const auto file =
			*params.save_path /
			fs::path("scores." + std::to_string(population->m_Generation))
		;

		std::ofstream scores(
			file.c_str(), std::ofstream::app | std::ofstream::out
		);
		if(scores.good()) {
			scores << step << " " << fitness << "\n";
		}
	}

	if(step+1 < params.population_size) {
		distribute_genomes_shared(step+1);
	}

	agents_in_goal = 0;
}

void herbivore_neat::epoch_shared(int epoch) {
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
		    *widget, new door_herbivore_widget::epoch_event(
				epoch,
				total / params.population_size,
				best_score,
				worst_score
			)
		);
	}

	if(params.train && params.save_path)
		save(total / params.population_size, best_score, worst_score, 0.0);

	if(params.train) {
		population->Epoch();
		fill_genome_vector();
	}
	distribute_genomes_shared(0);
}

void herbivore_neat::epoch_normal(int epoch, int steps) {
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
		    new door_herbivore_widget::epoch_event(
				epoch,
				total/agents.size(),
				best_score / static_cast<double>(steps),
				agents_in_goal / static_cast<double>(steps)
			)
		);
	}

	if(params.train && params.save_path)
		save(total / params.population_size, best_score, worst_score, agents_in_goal / static_cast<double>(steps));

	if(params.train) {
		population->Epoch();
		distribute_genomes();
	}
	agents_in_goal = 0;
}

QWidget *herbivore_neat::make_species_widget() {
	return new door_herbivore_widget(this, params.avg_window);
}

void herbivore_neat::save(double avg, double high, double low, double agents) const {
	if(!params.save_path) return;
	save_neat_population(
		*params.save_path / fs::path(std::to_string(population->m_Generation)),
		*population
	);

	std::ofstream scores(
		(*params.save_path / fs::path("scores")).c_str(),
		std::ofstream::app | std::ofstream::out
	);

	if(scores) {
		scores <<
			population->m_Generation << " " <<
			high << " " << low  << " " << avg  <<  " " << agents << '\n'
		;
	}
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
			case fixture_type::wall_goal:
				return &vision_goal;
			case fixture_type::wall_button:
				return &vision_button;
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
	if(std::abs(diff_angle) > M_PI_2)
		return;
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
		} else if(native_fixture_type == fixture_type::torso && foreign_fixture_type == fixture_type::wall_goal) {
			score +=  100;
			body->SetActive(false);
			active = false;
			species->agents_in_goal++;
			species->tick_goal_count++;
		} else if(native_fixture_type == fixture_type::torso && foreign_fixture_type == fixture_type::wall_button) {
			is_on_button = true;
			species->agents_on_buttons.emplace_back(this);
			species->env.set_button_active(0);
		}
	} else if(type == typeid(msg_plot)) {
		draw_vision = !draw_vision;
		plot_genome(*genotype, "selected_agent");
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
