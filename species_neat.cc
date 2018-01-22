#include <Genome.h>
#include <Parameters.h>
#include <NeuralNetwork.h>

#include "species_neat.h"
#include "body.h"

#include <random>
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

};
