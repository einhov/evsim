#include <Genome.h>
#include <Population.h>

#include "neat_plot.h"

namespace evsim {

void plot_genome(const NEAT::Genome &genome, const char* filename, const NEAT::Population *population) {
	constexpr bool group_nodes = false;
	constexpr bool display_weights = true;
	const double best_fitness = population->GetBestFitnessEver();
	const double fitness = genome.GetFitness();
	const unsigned int generation = population->GetGeneration();
	const int input_count = genome.NumInputs();
	const int output_count = genome.NumOutputs();
	const auto &links = genome.m_LinkGenes;
	std::set<int> hidden_nodes;
	FILE *plotdata = fopen(filename, "w");

	fprintf(plotdata, "digraph graphname {\n");
	fprintf(
		plotdata,
		"\tgraph [label=\"Showing the best Genome in the generation\n"
		"Fitness: %3.2f\n"
		"Generation: %d\t bestFitnessEver: %3.2f \", labelloc=t, fontsize=30];\n",
		fitness, generation, best_fitness
	);

	for (unsigned int i = 0; i < genome.NumLinks(); i++) {
		int fromId = links[i].FromNeuronID();
		if(fromId > input_count+output_count) {
			set<int>::iterator it = hidden_nodes.find(fromId);
			if (it == hidden_nodes.end()) {
				hidden_nodes.insert(fromId);
			}
		}
		if(display_weights) {
			fprintf(
				plotdata,
				"\t%d -> %d [label=%3.2f,weight=%3.2f];\n",
				fromId, links[i].ToNeuronID(),
				links[i].GetWeight(), links[i].GetWeight()
			);
		}
		else {
			fprintf(plotdata, "\t%d -> %d;\n", fromId, links[i].ToNeuronID());
		}
	}
	for(int i = 1; i <= input_count; i++) {
		fprintf(plotdata, "\t%d [shape=circle, style=filled, fillcolor=green]\n", i);
	}
	for(int i = input_count + 1; i <= input_count + output_count; i++) {
		fprintf(plotdata, "\t%d [shape=circle, style=filled, fillcolor=red]\n", i);
	}
	if(group_nodes){
		fprintf(plotdata, "\t{ rank=same; ");
		for(int i = 1; i <= input_count; i++) {
			fprintf(plotdata, "%d", i );
			if (i < input_count) {
				fprintf(plotdata,", ");
			}
		}
		fprintf(plotdata, "}\n");
		fprintf(plotdata, "\t{ rank=same; ");
		for (auto current = hidden_nodes.begin(); current != hidden_nodes.end();) {
			fprintf(plotdata, "%d", (*current));
			if (++current != hidden_nodes.end()) {
				fprintf(plotdata,", ");
			}
		}
		fprintf(plotdata, "}\n");
		fprintf(plotdata, "\t{ rank=same; ");
		for(int i = input_count + 1; i <= input_count + output_count; i++) {
			fprintf(plotdata, "%d ", i );
			if (i+1 < input_count+output_count) {
				fprintf(plotdata,", ");
			}
		}
		fprintf(plotdata, "}\n");
	}
	fprintf(plotdata, "\n}");
	fclose(plotdata);
}

}
