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
	steps_per_generation = 10,
	ticks_per_step = 60 * 15,

	herbivores = {
		population_size = 100,
		training_model = "normal", --"none", "normal", "shared"
		thrust = 1000.0,
		torque = 45.0,
		shared_fitness_simulate_count = 5,

		neat_params = neat_params
	}
}

multi_food = {
	name = "multi_food",
	food_count = 150,
	steps_per_generation = 50,
	ticks_per_step = 60 * 15,

	herbivores = {
		population_size = 100,
		training_model = "normal", --"none", "normal", "shared"
		thrust = 1000.0,
		torque = 45.0,
		yell_delay = 30,
		shared_fitness_simulate_count = 5,

		neat_params = neat_params
	},

	predators = {
		population_size = 100,
		training_model = "normal", --"none", "normal", "shared"
		thrust = 1000.0,
		torque = 45.0,
		eat_delay = 60, -- < 0: once, == 0: no_delay, > 0: delay
		shared_fitness_simulate_count = 5,

		neat_params = neat_params
	}
}

multi_move = {
	name = "multi_move",
	steps_per_generation = 10,
	ticks_per_step = 60 * 15,

	herbivores = {
		population_size = 50,
		training_model = "normal", --"none", "normal", "shared"
		thrust = 1000.0,
		torque = 45.0,
		shared_fitness_simulate_count = 5,

		neat_params = neat_params
	},

	predators = {
		population_size = 50,
		training_model = "normal", --"none", "normal", "shared"
		thrust = 1000.0,
		torque = 45.0,
		eat_delay = 60, -- < 0: once, == 0: no_delay, > 0: delay
		shared_fitness_simulate_count = 5,

		neat_params = neat_params
	}
}

environments = {
	food = food,
	multi_food = multi_food,
	multi_move = multi_move
}

environment = environments[arg[2]] or food
