#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <ctime>
#include <cstdlib> //is this needed? check
#include "datatypes.h"
#include "fuzzy.h"

using namespace std;

vector<string> rules;
const string operators[] = {"AND", "OR"};
vector<Edge> edges;
vector<GA_node> input_nodes;
vector<GA_node> output_nodes;

void GA_add_edge(GA_node input_node, GA_node output_node){
	Edge edge;
	edge.input_node = input_node;
	edge.output_node = output_node;
	edge.input_value_index = rand()%input_node.values.size();
	edge.output_value_index = rand()%output_node.values.size();
	edges.push_back(edge);
}

void GA_change_weight_value(Edge &edge){
	if(rand() % 2 == 0) {
		edge.input_value_index = rand()%edge.input_node.values.size();
	}
	else {
		edge.output_value_index = rand()%edge.output_node.values.size();
	}
}

vector<string> GA_generate_rules(){
	vector<string> rules;
	for(int i = 0; i < edges.size(); i++){
		Edge &edge = edges[i];
		string input_node = edge.input_node.name;
		string output_node = edge.output_node.name;
		string input_node_value = edge.input_node.values[edge.input_value_index];
		string output_node_value = edge.output_node.values[edge.output_value_index];
		string rule = "if " + input_node + " is " + input_node_value + " then " +
				output_node + " is " + edge.output_node.values[edge.output_value_index];
		rules.push_back(rule);
	}
	return rules;
}
vector<string> GA_configure(GA_data_in data){
	/* initialize random seed: */
	srand (time(NULL));

	int population_size = 20;
	double mutation_chance = 0.05;
	double crossover_chance = 0.01;

	input_nodes = data.input_nodes;
	output_nodes = data.output_nodes;

	for(int i = 0; i < input_nodes.size(); i++){
		for(int j = 0; j < output_nodes.size(); j++){
			//add_edge(input_nodes[i], output_nodes[rand()%output_nodes.size()]);
			GA_add_edge(input_nodes[i], output_nodes[j]);
		}
	}

	vector<string> rules = GA_generate_rules();
	return rules;
}

void GA_mutate(){
	for(int i = 0; i < edges.size(); i++){
		GA_change_weight_value(edges[i]);
	}
	//TODO
	//add_node
	//change input value
	//change output value
	// remove edge
	//change operator
	//add operator
	//remove operator
}



void crossover(){;}

void GA_run(){
	GA_mutate();
	vector<string> result_rules = GA_generate_rules();
	fuzzy_set_rulebook(result_rules);
}

void GA_init(int sensor_size){
	GA_data_in data;
	vector<GA_node> in (2*sensor_size+2);
	vector<GA_node> out (2);
	for(int i = 0; i < sensor_size; i++){
		GA_node &food = in[i*2];
		food.name = "food" + std::to_string(i);
		food.values.emplace_back("false");
		food.values.emplace_back("far");
		food.values.emplace_back("near");
		GA_node &poison = in[i*2+1];
		poison.name = "poison" + std::to_string(i);
		poison.values.emplace_back("false");
		poison.values.emplace_back("far");
		poison.values.emplace_back("near");
	}
	GA_node &linear_velocity_in = in[in.size() -2];
	linear_velocity_in.name = "linear_velocity_in";
	linear_velocity_in.values.emplace_back("left");
	linear_velocity_in.values.emplace_back("center");
	linear_velocity_in.values.emplace_back("right");

	GA_node &angular_velocity_in = in[in.size() -1];
	angular_velocity_in.name = "angular_velocity_in";
	angular_velocity_in.values.emplace_back("left");
	angular_velocity_in.values.emplace_back("center");
	angular_velocity_in.values.emplace_back("right");

	GA_node &linear_velocity_out = out[0];
	linear_velocity_out.name = "linear_velocity_out";
	linear_velocity_out.values.emplace_back("none");
	linear_velocity_out.values.emplace_back("slow");
	linear_velocity_out.values.emplace_back("medium");
	linear_velocity_out.values.emplace_back("fast");
	linear_velocity_out.values.emplace_back("hyperspeed");

	GA_node &angular_velocity_out = out[1];
	angular_velocity_out.name = "angular_velocity_out";
	angular_velocity_out.values.emplace_back("hard_left");
	angular_velocity_out.values.emplace_back("left");
	angular_velocity_out.values.emplace_back("center");
	angular_velocity_out.values.emplace_back("right");
	angular_velocity_out.values.emplace_back("hard_right");
	data.input_nodes = in;
	data.output_nodes = out;

	vector<string> rules = GA_configure(data);
	fuzzy_init(sensor_size);
	fuzzy_set_rulebook(rules);
}
