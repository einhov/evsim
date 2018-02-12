#include <Genome.h>
#include <Population.h>

#include "neat_plot.h"

namespace evsim {

void plot_genome(const NEAT::Genome &genome, const char* filename) {
	constexpr bool group_nodes = false;
	constexpr bool display_weights = true;
	const double fitness = genome.GetFitness();
	const int input_count = genome.NumInputs();
	const int output_count = genome.NumOutputs();
	const auto &links = genome.m_LinkGenes;
	std::set<int> hidden_nodes;
	FILE *plotdata = fopen(filename, "w");

	fprintf(plotdata, "digraph graphname {\n");
	fprintf(
		plotdata,
		"\tgraph [label=\"Showing the genotype of the selected agent\",\n"
				"\t\tlabelloc=t, fontsize=30];\n",
		fitness
	);

	for(const auto &link : genome.m_LinkGenes) {
		int from = link.FromNeuronID();
		int to   = link.ToNeuronID();
		if(from > input_count + output_count) {
			set<int>::iterator it = hidden_nodes.find(from);
			if (it == hidden_nodes.end()) {
				hidden_nodes.insert(from);
			}
		}
		fprintf(plotdata, "\t%d -> %d", from, to);
		if(display_weights) {
			fprintf(
				plotdata, " [label=%3.2f,weight=%3.2f]", link.GetWeight(), link.GetWeight()
			);
		}
		fputs(";\n", plotdata);
	}
	for(int i = 1; i <= input_count; i++) {
		fprintf(plotdata, "\t%d [shape=circle, style=filled, fillcolor=green]\n", i);
	}
	for(int i = input_count + 1; i <= input_count + output_count; i++) {
		fprintf(plotdata, "\t%d [shape=circle, style=filled, fillcolor=red]\n", i);
	}
	if constexpr(group_nodes){
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
