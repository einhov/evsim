#ifndef NEAT_PLOT_H
#define NEAT_PLOT_H

#include <Genome.h>
#include <Population.h>

namespace evsim {

void plot_genome(const NEAT::Genome &genome, const char* filename, const NEAT::Population *population);

}

#endif
