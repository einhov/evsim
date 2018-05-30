#ifndef ENV_ENVIRONMENTS_H
#define ENV_ENVIRONMENTS_H

#include <memory>

#include "food/environment.h"
#include "multi_food/environment.h"
#include "multi_move/environment.h"
#include "door/environment.h"

#include "../lua_conf.h"
#include "../environment_base.h"

namespace evsim {

	std::unique_ptr<environment_base> make_environment(lua_conf &conf);

}
 
#endif
