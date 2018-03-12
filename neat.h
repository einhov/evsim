#include <Parameters.h>
#include "lua_conf.h"

namespace evsim {

inline NEAT::Parameters make_neat_params(lua_conf &conf) {
	NEAT::Parameters params;
	params.MinSpecies = conf.get_integer_default("min_species", 3);
	params.MaxSpecies = conf.get_integer_default("max_species", 20);
	params.CompatTreshold = conf.get_number_default("compat_thresh", 5.0);
	params.MutateNeuronActivationTypeProb = 0.5;
	return params;
}

}
