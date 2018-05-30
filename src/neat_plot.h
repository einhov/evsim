#ifndef NEAT_PLOT_H
#define NEAT_PLOT_H

#include <Genome.h>
#include <Population.h>

namespace evsim {

struct msg_plot {
};

void plot_genome(const NEAT::Genome &genome, const char* filename);

}

#endif
