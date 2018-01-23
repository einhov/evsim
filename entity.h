#ifndef ENTITY_H
#define ENTITY_H

#include <any>

namespace evsim {

class entity {
	public:
		virtual void message(const std::any &msg) = 0;
};

};

#endif
