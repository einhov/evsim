#include <random>
#include <iterator>
#include <algorithm>
#include <utility>
#include <optional>

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

#include "herbivore_neat.h"

namespace evsim {
namespace multi_food {

static std::default_random_engine generator(std::random_device{}());
static std::uniform_real_distribution<float> pos_x_distribution(-99.0f * (4.0f / 3.0f), 99.0f * (4.0f / 3.0f));
static std::uniform_real_distribution<float> pos_y_distribution(-99.0f, 99.0f);

void herbivore_neat::clear() {
	for(const auto &agent : agents)
		world.DestroyBody(agent.body);
	agents.clear();
	population.release();
	population_size = 0;
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
			if(++n >= population_size) return;
		}
		s++;
	}
}

bool herbivore_neat::initialise(size_t size, int seed) {
	if(population_size > 0)
		clear();

	conf.draw_sensors_herbivore = true;
	population_size = size;
	agents.resize(population_size);

	static std::default_random_engine generator;
	static std::uniform_real_distribution<float> pos_x_distribution(-99.0f * (4.0f / 3.0f), 99.0f * (4.0f / 3.0f));
	static std::uniform_real_distribution<float> pos_y_distribution(-99.0f, 99.0f);
	for(auto &agent : agents) {
		agent.body = build_body(world);
		agent.body->SetTransform(
			b2Vec2(pos_x_distribution(generator), pos_y_distribution(generator)),
			agent.body->GetAngle()
		);
		agent.body->SetUserData(reinterpret_cast<void*>(&agent));
		agent.species = this;
	}

	NEAT::Parameters params;
	params.PopulationSize = population_size;
	params.MinSpecies = build_config::hv_min_species;
	params.MaxSpecies = build_config::hv_max_species;
	params.CompatTreshold = build_config::hv_compat_treshold;

	NEAT::Genome genesis(
		0, 6 + agent::vision_segments * 3, 0, 3, false,
		NEAT::SIGNED_SIGMOID, NEAT::SIGNED_SIGMOID,
		0, params, 0
	);

	population = std::make_unique<NEAT::Population>(genesis, params, true, 1.0, seed);
	distribute_genomes();
}

void herbivore_neat::pre_tick() {
	for(auto &agent : agents) {
		agent.vision_food = {};
		agent.vision_herbivore = {};
		agent.vision_predator = {};
	}
}

void herbivore_neat::tick() {
	for(auto &agent : agents) {
		auto &body = agent.body;
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
			agent.vision_food.cbegin(), agent.vision_food.cend(),
			std::back_inserter(inputs), vision_inserter
		);
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
		agent.hear_yell = false;
		inputs.emplace_back(1.0);

		agent.phenotype.Flush();
		agent.phenotype.Input(inputs);
		agent.phenotype.Activate();
		const auto output = agent.phenotype.Output();

		const auto forward =
			glm::rotate(glm::vec2 { 0.0f, 1.0f }, angle) *
			static_cast<float>(output[0]) *
			build_config::hv_force
		;

		body->ApplyForceToCenter(b2Vec2 { forward.x, forward.y }, true);
		body->ApplyTorque(output[1] * build_config::hv_torque, true);
		if(output[2] >= 0.1) {
			agent.create_yell();
		}
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
	}
	fprintf(stderr, "NEAT :: Average score: %lf\n", total / agents.size());
}

void herbivore_neat::epoch(int steps) {
	for(auto &agent : agents) {
		agent.genotype->SetFitness(agent.generation_score / static_cast<double>(steps));
		agent.genotype->m_Evaluated = true;
		agent.generation_score = 0;
	}
	fprintf(stderr, "NEAT :: Best genotype: %lf\n", population->GetBestGenome().GetFitness());
	population->Epoch();
	fprintf(stderr, "NEAT :: Best ever    : %lf\n", population->GetBestFitnessEver());
	fprintf(stderr, "NEAT :: Species: %zu\n", population->m_Species.size());
	distribute_genomes();
}

static void relocate_agent(b2Body *body) {
	body->SetTransform(b2Vec2(pos_x_distribution(generator), pos_y_distribution(generator)), 0.0f);
}

void herbivore_neat::agent::on_sensor(const msg_contact &contact) {
	using vt = std::array<float, vision_segments>;
	const auto vision_texture = [this,&contact]() -> std::optional<vt*> {
		const auto &foreign_userdata = contact.fixture_foreign->GetUserData();
		const auto &native_fixture_type = *static_cast<fixture_type*>(foreign_userdata);
		switch(native_fixture_type) {
			case fixture_type::food:
				return &vision_food;
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
	const auto &type = msg.type();
	if(type == typeid(msg_contact)) {
		const auto &contact = std::any_cast<msg_contact>(msg);
		const auto &native_userdata = contact.fixture_native->GetUserData();
		const auto &native_fixture_type = *static_cast<fixture_type*>(native_userdata);
		const auto &foreign_userdata = contact.fixture_foreign->GetUserData();
		const auto &foreign_fixture_type = *static_cast<fixture_type*>(foreign_userdata);

		if(native_fixture_type == fixture_type::sensor) {
			on_sensor(contact);
		} else if(native_fixture_type == fixture_type::torso && foreign_fixture_type == fixture_type::food) {
			const auto &food = static_cast<entity*>(contact.fixture_foreign->GetBody()->GetUserData());
			food->message(std::make_any<msg_consume>(msg_consume { this }));
		} else if(native_fixture_type == fixture_type::torso && foreign_fixture_type == fixture_type::yell) {
			const yell *yell_heard = static_cast<yell*>(contact.fixture_foreign->GetBody()->GetUserData());
			if(yell_heard->hollerer != this) {
				hear_yell = true;
				centre_of_yell = yell_heard->body->GetPosition();
			}
		}
	} else if(type == typeid(msg_consumed)) {
		score++;
	} else if(type == typeid(msg_kill)) {
		const auto &consumer = std::any_cast<msg_kill>(msg).consumer;
		score -= 2;
		relocate_agent(body);
		consumer->message(std::make_any<msg_killed>());
	} else if(type == typeid(msg_plot)) {
		plot_genome(*genotype, "selected_agent");
	}
}

void herbivore_neat::agent::create_yell() {
	if(can_yell_timer == 0){
		can_yell_timer = yell_timer_max;
		auto yell_instance = std::make_unique<yell>();
		yell_instance->init_body(species->world, this);
		environmental_objects.push_back(std::move(yell_instance));
	}
}

}
}
