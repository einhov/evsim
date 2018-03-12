-- Example configuration file

-- Contains (or should contain) an exhaustive listing of parameters

neat_params = {
	min_species = 3,
	max_species = 20,
	compat_thresh = 5.0
}

food = {
	name = "food",
	food_count = 150,

	herbivores = {
		population_size = 100,
		thrust = 1000.0,
		torque = 45.0,

		neat_params = neat_params
	}
}

multi_food = {
	name = "multi_food",
	food_count = 150,

	herbivores = {
		population_size = 100,
		thrust = 1000.0,
		torque = 45.0,
		yell_delay = 30,

		neat_params = neat_params
	},

	predators = {
		population_size = 100,
		thrust = 1000.0,
		torque = 45.0,

		neat_params = neat_params
	}
}

multi_move = {
	name = "multi_move",

	herbivores = {
		population_size = 50,
		thrust = 1000.0,
		torque = 45.0,

		neat_params = neat_params
	},

	predators = {
		population_size = 50,
		thrust = 1000.0,
		torque = 45.0,

		neat_params = neat_params
	}
}

environments = {
	food = food,
	multi_food = multi_food,
	multi_move = multi_move
}

environment = environments[arg[2]] or food
