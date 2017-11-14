#include <fl/Headers.h>
#include <iostream>
using namespace fl;

Engine* engine;
InputVariable* obstacle;
InputVariable* food;
OutputVariable* mSteer;

int fuzzy_init(){

	engine = new Engine;
	  engine->setName("ObstacleAvoidance");
	  engine->setDescription("");

	  obstacle = new InputVariable;
	  obstacle->setName("obstacle");
	  obstacle->setDescription("");
	  obstacle->setEnabled(true);
	  obstacle->setRange(0.000, 1.000);
	  obstacle->setLockValueInRange(false);
	  obstacle->addTerm(new Trapezoid("false", 0.000, 0.000, 0.250, 0.550));
	  //obstacle->addTerm(new Triangle("center", 0.250, 0.5, 0.750));
	  obstacle->addTerm(new Trapezoid("true", 0.450, 0.750, 1.000, 1.000));
	  engine->addInputVariable(obstacle);

	  food = new InputVariable;
	  food->setName("food");
	  food->setDescription("");
	  food->setEnabled(true);
	  food->setRange(0.000, 1.000);
	  food->setLockValueInRange(false);
	  food->addTerm(new Trapezoid("left", 0.000, 0.000, 0.250, 0.550));
	  food->addTerm(new Triangle("center", 0.250, 0.5, 0.750));
	  food->addTerm(new Trapezoid("right", 0.450, 0.750, 1.000, 1.000));
	  engine->addInputVariable(food);

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
	  mSteer->addTerm(new Trapezoid("turn", 0.000, 0.000, 0.250, 0.550));
	  mSteer->addTerm(new Trapezoid("gas", 0.450, 0.750, 1.000, 1.000));
	  //mSteer->addTerm(new Trapezoid("left", 0.000, 0.000, 0.250, 0.550));
	  //mSteer->addTerm(new Triangle("front", 0.250, 0.500, 0.750));
	  //mSteer->addTerm(new Trapezoid("right", 0.450, 0.750, 1.000, 1.000));
	  engine->addOutputVariable(mSteer);

	  RuleBlock* mamdani = new RuleBlock;
	  mamdani->setName("mamdani");
	  mamdani->setDescription("");
	  mamdani->setEnabled(true);
	  mamdani->setConjunction(fl::null);
	  mamdani->setDisjunction(fl::null);
	  mamdani->setImplication(new AlgebraicProduct);
	  mamdani->setActivation(new General);
	  mamdani->addRule(Rule::parse("if obstacle is false then mSteer is turn", engine));
	  mamdani->addRule(Rule::parse("if obstacle is true then mSteer is gas", engine));
	  //mamdani->addRule(Rule::parse("if obstacle is left then mSteer is right", engine));
	  //mamdani->addRule(Rule::parse("if obstacle is right then mSteer is left", engine));
	  //mamdani->addRule(Rule::parse("if food is left then mSteer is left", engine));
	  //mamdani->addRule(Rule::parse("if food is right then mSteer is right", engine));
	  engine->addRuleBlock(mamdani);

	  std::string status;
	  if (not engine->isReady(&status))
		  throw Exception("[engine error] engine is not ready:\n" + status, FL_AT);

	  obstacle->setValue(0.0);
	  food->setValue(0.0);
	  engine->process();
//	  FL_LOG("obstacle.input = " << Op::str(0.0) << " => " << "steer.output = "
//			 << Op::str(mSteer->getValue()) << "fuzzy output: " << mSteer->fuzzyOutputValue());

}

bool fuzzy_getAction(bool left, bool right){
	if (left)
		obstacle->setValue(1.0);
	else
		obstacle->setValue(0.0);
	engine->process();
	if(mSteer->getValue() >= 0.5){
		return true;
	}
	else
		return false;
}

/*
int main(int argc, char **argv){
	fuzzy_init();
}
*/
