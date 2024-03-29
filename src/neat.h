#ifndef NEAT_H
#define NEAT_H

#include <memory>
#include <boost/filesystem.hpp>

#include <Population.h>
#include <Parameters.h>
#include "lua_conf.h"

namespace evsim {

inline NEAT::Parameters make_neat_params(lua_conf &conf) {
	NEAT::Parameters params;
	params.DynamicCompatibility = conf.get_boolean_default("dynamic_compatibility", true);
	params.MinSpecies = conf.get_integer_default("min_species", 3);
	params.MaxSpecies = conf.get_integer_default("max_species", 20);
	params.CompatTreshold = conf.get_number_default("compat_thresh", 5.0);

	params.CrossoverRate = conf.get_number_default("crossover_rate", 0.7);
	params.OverallMutationRate = conf.get_number_default("mutation_rate", 0.25);
	params.TournamentSize = conf.get_integer_default("tournament_size", 4);
	params.EliteFraction = conf.get_number_default("elite_fraction", 0.01);
	params.OldAgePenalty = conf.get_number_default("old_age_penalty", 0.5);

	params.MutateNeuronActivationTypeProb = conf.get_number_default("MutateNeuronActivationTypeProb", 0.0);

	params.ActivationFunction_SignedSigmoid_Prob = conf.get_number_default("ActivationFunction_SignedSigmoid_Prob", 0.0);
	params.ActivationFunction_UnsignedSigmoid_Prob = conf.get_number_default("ActivationFunction_UnsignedSigmoid_Prob", 1.0);
	params.ActivationFunction_Tanh_Prob = conf.get_number_default("ActivationFunction_Tanh_Prob", 0.0);
	params.ActivationFunction_TanhCubic_Prob = conf.get_number_default("ActivationFunction_TanhCubic_Prob", 0.0);
	params.ActivationFunction_SignedStep_Prob = conf.get_number_default("ActivationFunction_SignedStep_Prob", 0.0);
	params.ActivationFunction_UnsignedStep_Prob = conf.get_number_default("ActivationFunction_UnsignedStep_Prob", 0.0);
	params.ActivationFunction_SignedGauss_Prob = conf.get_number_default("ActivationFunction_SignedGauss_Prob", 0.0);
	params.ActivationFunction_UnsignedGauss_Prob = conf.get_number_default("ActivationFunction_UnsignedGauss_Prob", 0.0);
	params.ActivationFunction_Abs_Prob = conf.get_number_default("ActivationFunction_Abs_Prob", 0.0);
	params.ActivationFunction_SignedSine_Prob = conf.get_number_default("ActivationFunction_SignedSine_Prob", 0.0);
	params.ActivationFunction_UnsignedSine_Prob = conf.get_number_default("ActivationFunction_UnsignedSine_Prob", 0.0);
	params.ActivationFunction_Linear_Prob = conf.get_number_default("ActivationFunction_Linear_Prob", 0.0);
	params.ActivationFunction_Relu_Prob = conf.get_number_default("ActivationFunction_Relu_Prob", 0.0);
	params.ActivationFunction_Softplus_Prob = conf.get_number_default("ActivationFunction_Softplus_Prob", 0.0);

	return params;
}

inline std::unique_ptr<NEAT::Population> load_neat_population(const boost::filesystem::path &file) {
	namespace fs = boost::filesystem;
	if(!fs::exists(file) || !fs::is_regular_file(file))
		throw std::runtime_error("Attempted to load invalid file");
	return std::make_unique<NEAT::Population>(file.c_str());
}

inline void save_neat_population(const boost::filesystem::path &file, NEAT::Population &population) {
	population.Save(file.c_str());
}

}

#endif
