#include <fl/Headers.h>
#include <iostream>
#include "fuzzy.h"
#include <vector>

using namespace fl;

Engine* engine;
InputVariable* obstacle;
std::vector<InputVariable*> food;
std::vector<InputVariable*> poison;
OutputVariable* mSteer;
OutputVariable* linearVelocity;
int sensor_size;

int fuzzy_init(int size){
	sensor_size = size;
	engine = new Engine;
	engine->setName("ObstacleAvoidance");
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

	linearVelocity = new OutputVariable;
	linearVelocity->setName("linearVelocity");
	linearVelocity->setDescription("");
	linearVelocity->setEnabled(true);
	linearVelocity->setRange(0.000, 1.000);
	linearVelocity->setLockValueInRange(false);
	linearVelocity->setAggregation(new Maximum);
	linearVelocity->setDefuzzifier(new Centroid(100));
	linearVelocity->setDefaultValue(fl::nan);
	linearVelocity->setLockPreviousValue(false);
	linearVelocity->addTerm(new Trapezoid("none", 0.000, 0.000, 0.125, 0.25));
	linearVelocity->addTerm(new Triangle("slow", 0.0, 0.25, 0.50));
	linearVelocity->addTerm(new Triangle("medium", 0.250, 0.5, 0.750));
	linearVelocity->addTerm(new Triangle("fast", 0.50, 0.75, 1.0));
	linearVelocity->addTerm(new Trapezoid("hyperspeed", 0.750, 0.875, 1.000, 1.000));
	engine->addOutputVariable(linearVelocity);

	mSteer = new OutputVariable;
	mSteer->setName("mSteer");
	mSteer->setDescription("");
	mSteer->setEnabled(true);
	mSteer->setRange(0.000, 1.000);
	mSteer->setLockValueInRange(false);
	mSteer->setAggregation(new Maximum);
	mSteer->setDefuzzifier(new Centroid(100));
	mSteer->setDefaultValue(fl::nan);
	mSteer->setLockPreviousValue(false);
	mSteer->addTerm(new Trapezoid("hard_left", 0.000, 0.000, 0.125, 0.25));
	mSteer->addTerm(new Triangle("left", 0.0, 0.25, 0.50));
	mSteer->addTerm(new Triangle("center", 0.250, 0.5, 0.750));
	mSteer->addTerm(new Triangle("right", 0.50, 0.75, 1.0));
	mSteer->addTerm(new Trapezoid("hard_right", 0.750, 0.875, 1.000, 1.000));
	engine->addOutputVariable(mSteer);

	RuleBlock* mamdani = new RuleBlock;
	mamdani->setName("mamdani");
	mamdani->setDescription("");
	mamdani->setEnabled(true);
	mamdani->setConjunction(new AlgebraicProduct);
	mamdani->setDisjunction(new AlgebraicSum);
	mamdani->setImplication(new AlgebraicProduct);
	mamdani->setActivation(new General);
	//food left
	mamdani->addRule(Rule::parse("if food0 is near then mSteer is hard_left", engine));
	mamdani->addRule(Rule::parse("if food0 is far then mSteer is left", engine));
	mamdani->addRule(Rule::parse("if food0 is false then mSteer is center", engine));

	//food right
	mamdani->addRule(Rule::parse("if food1 is near then mSteer is hard_right", engine));
	mamdani->addRule(Rule::parse("if food1 is far then mSteer is right", engine));
	mamdani->addRule(Rule::parse("if food1 is false then mSteer is center", engine));

	//food center
	mamdani->addRule(Rule::parse("if food0 is near and (food1 is near) then linearVelocity is hyperspeed", engine));
	mamdani->addRule(Rule::parse("if food0 is near and food1 is not near then linearVelocity is slow", engine));
	mamdani->addRule(Rule::parse("if food1 is near and food0 is not near then linearVelocity is slow", engine));
	mamdani->addRule(Rule::parse("if food0 is false then linearVelocity is fast", engine));
	mamdani->addRule(Rule::parse("if food0 is false then linearVelocity is fast", engine));

	//poison
	mamdani->addRule(Rule::parse("if poison0 is near then mSteer is hard_right", engine));
	mamdani->addRule(Rule::parse("if poison1 is near then mSteer is hard_left", engine));

	engine->addRuleBlock(mamdani);

	std::string status;
	if (not engine->isReady(&status))
		throw Exception("[engine error] engine is not ready:\n" + status, FL_AT);
}

force_increment fuzzy_getAction(agent_state state) {
	std::string check;
	 if (not engine->isReady(&check)){
		FL_LOG(check);
	 }

	for(int i = 0; i < sensor_size; i++){
		std::cout << state.sensor_food[i] << std::endl;
		food[i]->setValue(state.sensor_food[i]);
		poison[i]->setValue(0.0);
	}
	engine->process();
	std::cout << "S- Out : " << mSteer->getValue() << " Fuzzy output : " << mSteer->fuzzyOutputValue() << std::endl;
	std::cout << "LV Out : " << linearVelocity->getValue() << " Fuzzy output : " << linearVelocity->fuzzyOutputValue() << std::endl;

	force_increment force;
	force.linear_force = linearVelocity->getValue();
	force.angular_force = mSteer->getValue() - 0.5;
	return force;
}

/*
int main(int argc, char **argv){
	fuzzy_init(10);
}
*/
