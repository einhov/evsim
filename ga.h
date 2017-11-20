#ifndef GA_H
#define GA_H
#include <vector>
#include <string>

void GA_add_edge(GA_node input_node, GA_node output_node);
void GA_change_weight_value(Edge edge);
std::vector<std::string> GA_generate_rules();
std::vector<std::string> GA_configure(GA_data_in data);
void GA_mutate();
void GA_run();
void GA_init(int sensor_size);

#endif // GA_H
