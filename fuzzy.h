#ifndef FUZZY_H
#define FUZZY_H
#include "datatypes.h"
#include <Box2D/Box2D.h>


int fuzzy_init(int sensor_size);
force_increment fuzzy_getAction(agent_state state);

#endif // FUZZY_H
