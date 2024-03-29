cmake_minimum_required (VERSION 3.1)
project (evsim)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

add_executable (evsim
	src/evsim.cc
	src/gfx_program.cc
	src/consumable.cc
	src/body.cc
	src/neat_plot.cc
	src/yell.cc
	src/input.cc
	src/wall.cc
	src/lua_conf.cc
	src/env/environments.cc

	#Including food environment
	src/env/food/environment.cc
	src/env/food/herbivore_neat.cc
	src/env/food/herbivore_neat_draw.cc
	src/env/food/food_herbivore_widget.cc

	#Including multi_food environment
	src/env/multi_food/environment.cc
	src/env/multi_food/herbivore_neat.cc
	src/env/multi_food/herbivore_neat_draw.cc
	src/env/multi_food/multi_food_herbivore_widget.cc
	src/env/multi_food/predator_neat.cc
	src/env/multi_food/predator_neat_draw.cc
	src/env/multi_food/multi_food_predator_widget.cc

	#Including multi_move environment
	src/env/multi_move/environment.cc
	src/env/multi_move/herbivore_neat.cc
	src/env/multi_move/herbivore_neat_draw.cc
	src/env/multi_move/multi_move_herbivore_widget.cc
	src/env/multi_move/predator_neat.cc
	src/env/multi_move/predator_neat_draw.cc
	src/env/multi_move/multi_move_predator_widget.cc

	#Including door environment
	src/env/door/environment.cc
	src/env/door/herbivore_neat.cc
	src/env/door/herbivore_neat_draw.cc
	src/env/door/door_herbivore_widget.cc

	src/ui/gui.cc
	src/ui/fitness_graph.cc
)

set_property (TARGET evsim PROPERTY CXX_STANDARD 17)
target_compile_options(evsim PRIVATE -Wall)
target_compile_definitions(evsim PRIVATE -DUSE_BOOST_RANDOM -DGLM_ENABLE_EXPERIMENTAL)

find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glm REQUIRED)
find_package(Boost REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Charts REQUIRED)

add_subdirectory(lib/MultiNEAT)
include(lib/lua.cmake)

#git submodule Box2d has to be build first with "premake ifndef config=release"

set(BOX2D_INCLUDED_INCLUDE_DIR
    ${CMAKE_SOURCE_DIR}/lib/Box2D/Box2D/
)

set(BOX2D_INCLUDED_LIBRARY ${CMAKE_SOURCE_DIR}/lib/Box2D/Box2D/Build/gmake/bin/Release/libBox2D.a)

include_directories (SYSTEM ${GLEW_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR} ${BOX2D_INCLUDED_INCLUDE_DIR} ${GLM_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} lib/MultiNEAT/src)
target_include_directories (evsim PRIVATE ${CMAKE_SOURCE_DIR}/src/ui)
target_link_libraries (evsim ${GLEW_LIBRARIES} glfw ${OPENGL_gl_LIBRARY} ${BOX2D_INCLUDED_LIBRARY} MultiNEAT Qt5::Widgets Qt5::Charts lua)
