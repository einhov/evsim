#include <fl/Headers.h>
#include <iostream>
#include "fuzzy.h"
#include <vector>

using namespace fl;

Engine* engine;
InputVariable* obstacle;
InputVariable* linear_velocity_in;
InputVariable* angular_velocity_in;
std::vector<InputVariable*> food, poison;
OutputVariable* angular_velocity_out;
OutputVariable* linear_velocity_out;
RuleBlock* mamdani;
int sensor_size;

int fuzzy_init(int size){
	sensor_size = size;
	engine = new Engine;
	engine->setName("fuzzy_controller");
	engine->setDescription("");
	InputVariable* food_i;

	for(int i = 0; i < sensor_size; i++){
		food_i = new InputVariable;
		food_i->setName("food" + std::to_string(i));
		food_i->setDescription("");
		food_i->setEnabled(true);
		food_i->setRange(0.000, 1.000);
		food_i->setLockValueInRange(false);
		food_i->addTerm(new Trapezoid("false", 0.000, 0.000, 0.250, 0.550));
		food_i->addTerm(new Triangle("far", 0.250, 0.5, 0.750));
		food_i->addTerm(new Trapezoid("near", 0.450, 0.750, 1.000, 1.000));
		engine->addInputVariable(food_i);
		food.push_back(food_i);
	}

	InputVariable* poison_i;
	for(int i = 0; i < sensor_size; i++){
		poison_i = new InputVariable;
		poison_i->setName("poison" + std::to_string(i));
		poison_i->setDescription("");
		poison_i->setEnabled(true);
		poison_i->setRange(0.000, 1.000);
		poison_i->setLockValueInRange(false);
		poison_i->addTerm(new Trapezoid("false", 0.000, 0.000, 0.250, 0.550));
		poison_i->addTerm(new Triangle("far", 0.250, 0.5, 0.750));
		poison_i->addTerm(new Trapezoid("near", 0.450, 0.750, 1.000, 1.000));
		engine->addInputVariable(poison_i);
		poison.push_back(poison_i);
	}

	linear_velocity_in = new InputVariable;
	linear_velocity_in->setName("linear_velocity_in");
	linear_velocity_in->setDescription("");
	linear_velocity_in->setEnabled(true);
	linear_velocity_in->setRange(-1.000, 1.000);
	linear_velocity_in->setLockValueInRange(false);
	linear_velocity_in->addTerm(new Trapezoid("left", -0.000, -0.000, -0.5, 0));
	linear_velocity_in->addTerm(new Triangle("center", -0.500, 0, 0.500));
	linear_velocity_in->addTerm(new Trapezoid("right", 0, 0, 1.000, 1.000));
	engine->addInputVariable(linear_velocity_in);

	angular_velocity_in = new InputVariable;
	angular_velocity_in->setName("angular_velocity_in");
	angular_velocity_in->setDescription("");
	angular_velocity_in->setEnabled(true);
	angular_velocity_in->setRange(-1.000, 1.000);
	angular_velocity_in->setLockValueInRange(false);
	angular_velocity_in->addTerm(new Trapezoid("left", -1.000, -1.000, -0.5, 0));
	angular_velocity_in->addTerm(new Triangle("center", -0.500, 0, 0.500));
	angular_velocity_in->addTerm(new Trapezoid("right", 0, 0, 1.000, 1.000));
	engine->addInputVariable(angular_velocity_in);

	linear_velocity_out = new OutputVariable;
	linear_velocity_out->setName("linear_velocity_out");
	linear_velocity_out->setDescription("");
	linear_velocity_out->setEnabled(true);
	linear_velocity_out->setRange(0.000, 1.000);
	linear_velocity_out->setLockValueInRange(false);
	linear_velocity_out->setAggregation(new Maximum);
	linear_velocity_out->setDefuzzifier(new Centroid(100));
	//linear_velocity_out->setDefaultValue(fl::nan);
	linear_velocity_out->setDefaultValue(0.0);
	linear_velocity_out->setLockPreviousValue(false);
	linear_velocity_out->addTerm(new Trapezoid("none", 0.000, 0.000, 0.125, 0.25));
	linear_velocity_out->addTerm(new Triangle("slow", 0.0, 0.25, 0.50));
	linear_velocity_out->addTerm(new Triangle("medium", 0.250, 0.5, 0.750));
	linear_velocity_out->addTerm(new Triangle("fast", 0.50, 0.75, 1.0));
	linear_velocity_out->addTerm(new Trapezoid("hyperspeed", 0.750, 0.875, 1.000, 1.000));
	engine->addOutputVariable(linear_velocity_out);

	angular_velocity_out = new OutputVariable;
	angular_velocity_out->setName("angular_velocity_out");
	angular_velocity_out->setDescription("");
	angular_velocity_out->setEnabled(true);
	angular_velocity_out->setRange(-1.000, 1.000);
	angular_velocity_out->setLockValueInRange(false);
	angular_velocity_out->setAggregation(new Maximum);
	angular_velocity_out->setDefuzzifier(new Centroid(100));
	//angular_velocity_out->setDefaultValue(fl::nan);
	angular_velocity_out->setDefaultValue(0.0);
	angular_velocity_out->setLockPreviousValue(false);
	angular_velocity_out->addTerm(new Trapezoid("hard_left", -1.000, -1.000, -0.75, -0.5));
	angular_velocity_out->addTerm(new Triangle("left", -0.75, -0.5, -0.25));
	angular_velocity_out->addTerm(new Triangle("center", -0.4, 0, 0.4));
	angular_velocity_out->addTerm(new Triangle("right", 0.25, 0.5, 0.75));
	angular_velocity_out->addTerm(new Trapezoid("hard_right", 0.50, 0.75, 1.000, 1.000));
	engine->addOutputVariable(angular_velocity_out);

	mamdani = new RuleBlock;
	mamdani->setName("mamdani");
	mamdani->setDescription("");
	mamdani->setEnabled(true);
	mamdani->setConjunction(new AlgebraicProduct);
	mamdani->setDisjunction(new AlgebraicSum);
	mamdani->setImplication(new AlgebraicProduct);
	mamdani->setActivation(new General);
	engine->addRuleBlock(mamdani);
}

void fuzzy_set_rulebook(std::vector<std::string> rules){
	std::cout << mamdani->rules().size() << std::endl;
	for(auto rule : mamdani->rules()) {
		delete rule;
	}
	mamdani->rules().clear();

	for(int i = 0; i < rules.size(); i++){
		mamdani->addRule(Rule::parse(rules[i], engine));
	}
	engine->removeRuleBlock("mamdani");
	engine->addRuleBlock(mamdani);

	std::string status;
	if (not engine->isReady(&status))
		throw Exception("[engine error] engine is not ready:\n" + status, FL_AT);
}

Force_increment fuzzy_getAction(Agent_state state) {
	std::string check;
	 if (not engine->isReady(&check)){
		FL_LOG(check);
	 }

	for(int i = 0; i < sensor_size; i++){
		food[i]->setValue(state.sensor_food[i]);
		poison[i]->setValue(0.0);
		linear_velocity_in->setValue(0.0);
		angular_velocity_in->setValue(0.5);

	}
	engine->process();
	Force_increment force;
	force.linear_force = linear_velocity_out->getValue();
	force.angular_force = angular_velocity_out->getValue();
	return force;
}

