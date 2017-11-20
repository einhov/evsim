#ifndef FUZZY_H
#define FUZZY_H
#include "datatypes.h"
#include <Box2D/Box2D.h>


int fuzzy_init(int sensor_size);
Force_increment fuzzy_getAction(Agent_state state);
void fuzzy_set_rulebook(std::vector<std::string> rules);

#endif // FUZZY_H
