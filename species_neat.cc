#include <random>

#include <Genome.h>
#include <Parameters.h>
#include <NeuralNetwork.h>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "species_neat.h"
#include "body.h"
#include "consumable.h"
#include "neat_plot.h"

namespace evsim {

void species_neat::clear() {
	for(const auto &agent : agents)
		world.DestroyBody(agent.body);
	agents.clear();
	population.release();
	population_size = 0;
	active_genomes = 0;
}

void species_neat::distribute_genomes() {
	auto &n = active_genomes;
	n = 0;
	int s = 0;
	for(auto &species : population->m_Species) {
		for(auto &individual : species.m_Individuals) {
			agents[n].genotype = &individual;
			agents[n].species = s;
			individual.BuildPhenotype(agents[n].phenotype);
			if(++n >= population_size) return;
		}
		s++;
	}
}

bool species_neat::initialise(size_t size, int seed) {
	if(population_size > 0)
		clear();

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
	}

	NEAT::Parameters params;
	params.PopulationSize = population_size;
	params.MinSpecies = 3;
	params.MaxSpecies = 20;
	params.DontUseBiasNeuron = false;
	params.CompatTreshold = 0.1;

	NEAT::Genome genesis(
		0, 5, 0, 2, false,
		NEAT::SIGNED_SIGMOID, NEAT::SIGNED_SIGMOID,
		0, params, 0
	);

	population = std::make_unique<NEAT::Population>(genesis, params, true, 1.0, seed);
	distribute_genomes();
}

void species_neat::pre_tick() {
	for(auto &agent : agents) {
		agent.detected = {};
	}
}

void species_neat::tick() {
	for(auto &agent : agents) {
		auto &body = agent.body;
		const auto angle = body->GetAngle();
		const auto pos = body->GetPosition();

		if(pos.y < -100.0f) body->SetTransform(b2Vec2(pos.x, 100.0f), angle);
		if(pos.y > 100.0f) body->SetTransform(b2Vec2(pos.x, -100.0f), angle);
		if(pos.x < -100.0f * (4.0 / 3.0)) body->SetTransform(b2Vec2(100.0f * (4.0 / 3.0), pos.y), angle);
		if(pos.x > 100.0f * (4.0 / 3.0)) body->SetTransform(b2Vec2(-100.0f * (4.0 / 3.0), pos.y), angle);

		const std::vector<double> inputs {
			agent.detected[0] ? 1.0 : 0.0,
			agent.detected[1] ? 1.0 : 0.0,
			[&body] { auto vel = body->GetLinearVelocity(); return sqrt(vel.x * vel.x + vel.y * vel.y); }(),
			body->GetAngularVelocity(),
			1.0
		};
		agent.phenotype.Flush();
		agent.phenotype.Input(const_cast<std::vector<double>&>(inputs));
		agent.phenotype.Activate();
		const auto output = agent.phenotype.Output();

		const auto forward = glm::rotate(glm::vec2 { 0.0f, 1.0f }, angle) * static_cast<float>(output[0]) * 1000.0f;
		body->ApplyForceToCenter(b2Vec2 { forward.x, forward.y }, true);
		body->ApplyTorque(output[1] * glm::radians(3.0f * 360.0f * 5.0f), true);
	}
}

void species_neat::step() {
	double total = 0;
	for(auto &agent : agents) {
		total += agent.score;
		agent.generation_score += agent.score;
		agent.score = 0;
	}
	fprintf(stderr, "NEAT :: Average score: %lf\n", total / agents.size());
}

void species_neat::epoch(int steps) {
	for(auto &agent : agents) {
		agent.genotype->SetFitness(agent.generation_score / static_cast<double>(steps));
		agent.genotype->m_Evaluated = true;
		agent.generation_score = 0;
	}
	if(plot) {
		plot_best();
	}
	fprintf(stderr, "NEAT :: Best genotype: %lf\n", population->GetBestGenome().GetFitness());
	population->Epoch();
	fprintf(stderr, "NEAT :: Best ever    : %lf\n", population->GetBestFitnessEver());
	fprintf(stderr, "NEAT :: Species: %zu\n", population->m_Species.size());
	distribute_genomes();
}

void species_neat::agent::message(const std::any &msg) {
	const auto &type = msg.type();
	if(type == typeid(msg_contact)) {
		const auto &contact = std::any_cast<msg_contact>(msg);

		if(contact.fixture_foreign->GetUserData() != reinterpret_cast<void*>(0x900df00d))
			return;

		const auto &native_userdata = contact.fixture_native->GetUserData();
		assert(native_userdata != nullptr);
		const auto &native_fixture_type = *static_cast<fixture_type*>(native_userdata);
		if(native_fixture_type == fixture_type::sensor_left) {
			detected[0] = true;
		} else if(native_fixture_type == fixture_type::sensor_right) {
			detected[1] = true;
		} else if(native_fixture_type == fixture_type::torso) {
			const auto &food = static_cast<entity*>(contact.fixture_foreign->GetBody()->GetUserData());
			food->message(std::make_any<msg_consume>(msg_consume { this }));
		}
	} else if(type == typeid(msg_consumed)) {
		score++;
	}
}

void species_neat::plot_best() {
	const auto genome = population->GetBestGenome();
	plot_genome(genome, "agent_best", population.get());
}

}
