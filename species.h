#ifndef SPECIES_H
#define SPECIES_H

#include <QWidget>

namespace evsim {

struct species {
	virtual QWidget *make_species_widget() = 0;
	virtual ~species() {}
};

}

#endif
