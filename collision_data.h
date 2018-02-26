#ifndef COLLISION_DATA_H
#define COLLISION_DATA_H

enum class collision_types {
	HERBIVORE	=	0x0001,
	PREDATOR	=	0x0002,
	CONSUMABLE	=	0x0004,
	WALL		=	0x0008,
	SENSOR		=	0x0010,
	YELL		=	0x0020
};

#endif
