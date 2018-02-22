cmake_minimum_required (VERSION 3.1)
project (evsim)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

add_executable (evsim
	evsim.cc
	gfx_program.cc
	consumable.cc
	body.cc
	neat_plot.cc
	yell.cc
	input.cc

	#Including food environment
	env/food/environment.cc
	env/food/herbivore_neat.cc
	env/food/herbivore_neat_draw.cc

	#Including multi_food environment
	env/multi_food/environment.cc
	env/multi_food/herbivore_neat.cc
	env/multi_food/herbivore_neat_draw.cc
	env/multi_food/predator_neat.cc
	env/multi_food/predator_neat_draw.cc

	#Including multi_move environment
	env/multi_move/environment.cc
	env/multi_move/herbivore_neat.cc
	env/multi_move/herbivore_neat_draw.cc
	env/multi_move/predator_neat.cc
	env/multi_move/predator_neat_draw.cc

	ui/gui.cc
)

add_definitions(-DUSE_BOOST_RANDOM -DGLM_ENABLE_EXPERIMENTAL)
set_property (TARGET evsim PROPERTY CXX_STANDARD 17)

find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glm REQUIRED)
find_package(Boost REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Charts REQUIRED)

add_subdirectory(lib/MultiNEAT)

#git submodule Box2d has to be build first with "premake ifndef config=release"

set(BOX2D_INCLUDED_INCLUDE_DIR
    ${CMAKE_SOURCE_DIR}/Box2D/Box2D/
)

set(BOX2D_INCLUDED_LIBRARY ${CMAKE_SOURCE_DIR}/Box2D/Box2D/Build/gmake/bin/Release/libBox2D.a)

include_directories (${GLEW_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR} ${BOX2D_INCLUDED_INCLUDE_DIR} ${GLM_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} lib/MultiNEAT/src)
target_link_libraries (evsim ${GLEW_LIBRARIES} glfw ${OPENGL_gl_LIBRARY} ${BOX2D_INCLUDED_LIBRARY} MultiNEAT Qt5::Widgets Qt5::Charts)
