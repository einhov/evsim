local env = arg[2]
local shared_fitness_simulate_count = tonumber(arg[3])

local species0_training_model = arg[4]
local species0_population_size = tonumber(arg[5])
local species1_training_model = arg[6]
local species1_population_size = tonumber(arg[7])
local neat_param_index = tonumber(arg[8])

local storage = string.format(
	"overnight/%s-%s-%s %d-%d-%d",
	env,
	species0_training_model,
	species1_training_model,
	species0_population_size,
	species1_population_size,
	shared_fitness_simulate_count
)

local neat_params = {
	{ compat_thresh = 1.0 },
	{ compat_thresh = 10.0 },
	{ crossover_rate = 0.0 },
	{ mutation_rate = 0.05 },
	{ mutation_rate = 0.5 },
	{ old_age_penalty = 0.0 },
	{ elite_fraction = 0.5 }
}

draw = false
max_generations = 500

environment = {
	name = env,
	steps_per_generation = 10,
	ticks_per_step = 60 * 30,

	herbivores = {
		training_model = species0_training_model,
		train = true,
		population_size = species0_population_size,
		shared_fitness_simulate_count = shared_fitness_simulate_count,
		save = storage .. "/h",
		neat_params = neat_params[neat_param_index]
	},

	predators = {
		training_model = species1_training_model,
		train = true,
		population_size = species1_population_size,
		shared_fitness_simulate_count = shared_fitness_simulate_count,
		save = storage .. "/p"
	}
}

if species0_training_model == "shared" then
	environment.steps_per_generation = species0_population_size
elseif species1_training_model == "shared" then
	environment.steps_per_generation = species1_population_size
end

sensors = {
	length = 120,
	fov = 45
}
