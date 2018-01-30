#ifndef ENTITY_H
#define ENTITY_H

#include <any>

namespace evsim {

class entity {
	public:
		virtual void message(const std::any &msg) = 0;
};

struct msg_contact {
	b2Fixture *fixture_native;
	b2Fixture *fixture_foreign;
};

};

#endif
