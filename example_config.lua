-- Example configuration file

-- Contains (or should contain) an exhaustive listing of parameters

max_generations = 50

neat_params = {
	dynamic_compatibility = true,
	min_species = 3,
	max_species = 20,
	compat_thresh = 5.0,
	crossover_rate = 0.7,
	mutation_rate = 0.25,
	tournament_size = 4,
	elite_fraction = 0.01,
	old_age_penalty = 0.5,

	MutateNeuronActivationTypeProb = 0.0,

	ActivationFunction_SignedSigmoid_Prob = 0.0,
	ActivationFunction_UnsignedSigmoid_Prob = 1.0,
	ActivationFunction_Tanh_Prob = 0.0,
	ActivationFunction_TanhCubic_Prob = 0.0,
	ActivationFunction_SignedStep_Prob = 0.0,
	ActivationFunction_UnsignedStep_Prob = 0.0,
	ActivationFunction_SignedGauss_Prob = 0.0,
	ActivationFunction_UnsignedGauss_Prob = 0.0,
	ActivationFunction_Abs_Prob = 0.0,
	ActivationFunction_SignedSine_Prob = 0.0,
	ActivationFunction_UnsignedSine_Prob = 0.0,
	ActivationFunction_Linear_Prob = 0.0,
	ActivationFunction_Relu_Prob = 0.0,
	ActivationFunction_Softplus_Prob = 0.0
}

food = {
	name = "food",
	food_count = 150,
	steps_per_generation = 10,
	ticks_per_step = 60 * 15,

	herbivores = {
		population_size = 100,
		training_model = "normal", -- "normal", "normal_none", "shared", "shared_none"
		thrust = 1000.0,
		torque = 45.0,
		shared_fitness_simulate_count = 5,
		save = nil, -- path to directory for storing the population every generation
		initial_population = nil, -- path to file with initial population
		avg_window = 21, -- size of window used for moving average in plot

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
		training_model = "normal", -- "normal", "normal_none", "shared", "shared_none"
		thrust = 1000.0,
		torque = 45.0,
		yell_delay = 30,
		shared_fitness_simulate_count = 5,
		save = nil, -- path to directory for storing the population every generation
		initial_population = nil, -- path to file with initial population
		avg_window = 21, -- size of window used for moving average in plot

		neat_params = neat_params
	},

	predators = {
		population_size = 100,
		training_model = "normal", -- "normal", "normal_none", "shared", "shared_none"
		thrust = 1000.0,
		torque = 45.0,
		eat_delay = 60, -- < 0: once, == 0: no_delay, > 0: delay
		shared_fitness_simulate_count = 5,
		save = nil, -- path to directory for storing the population every generation
		initial_population = nil, -- path to file with initial population
		avg_window = 21, -- size of window used for moving average in plot

		neat_params = neat_params
	}
}

multi_move = {
	name = "multi_move",
	steps_per_generation = 10,
	ticks_per_step = 60 * 15,

	herbivores = {
		population_size = 50,
		training_model = "normal", -- "normal", "normal_none", "shared", "shared_none"
		thrust = 1000.0,
		torque = 45.0,
		shared_fitness_simulate_count = 5,
		save = nil, -- path to directory for storing the population every generation
		initial_population = nil, -- path to file with initial population
		avg_window = 21, -- size of window used for moving average in plot

		neat_params = neat_params
	},

	predators = {
		population_size = 50,
		training_model = "normal", -- "normal", "normal_none", "shared", "shared_none"
		thrust = 1000.0,
		torque = 45.0,
		eat_delay = 60, -- < 0: once, == 0: no_delay, > 0: delay
		shared_fitness_simulate_count = 5,
		save = nil, -- path to directory for storing the population every generation
		initial_population = nil, -- path to file with initial population
		avg_window = 21, -- size of window used for moving average in plot

		neat_params = neat_params
	}
}

door = {
	name = "door",
	steps_per_generation = 1,
	ticks_per_step = 60 * 15,

	herbivores = {
		population_size = 50,
		training_model = "normal", -- "normal", "normal_none", "shared", "shared_none"
		thrust = 1000.0,
		torque = 45.0,
		shared_fitness_simulate_count = 5,
		save = nil, -- path to directory for storing the population every generation
		initial_population = nil, -- path to file with initial population
		avg_window = 21, -- size of window used for moving average in plot

		neat_params = neat_params
	}
}

environments = {
	food = food,
	multi_food = multi_food,
	multi_move = multi_move,
	door = door
}

physics = {
	linear_damping = 10.0,
	angular_damping = 10.0
}

sensors = {
	length = 45.0,
	fov = 60.0
}

environment = environments[arg[2]] or food
