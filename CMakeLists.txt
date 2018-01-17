cmake_minimum_required (VERSION 2.6)
project (evsim)

add_executable (evsim evsim.cc gfx_program.cc)
add_definitions (-DUSE_BOOST_RANDOM)
set_property (TARGET evsim PROPERTY CXX_STANDARD 17)

find_package (glfw3 REQUIRED)
find_package (GLEW REQUIRED)
find_package (OpenGL REQUIRED)
find_package (glm REQUIRED)
find_package (Boost REQUIRED)

add_subdirectory(lib/MultiNEAT)

#git submodule Box2d has to be build first with premake.

set(_BOX2D_INCLUDED_INCLUDE_DIRS = "
    Box2D/Box2D/Box2D;
    Box2D/Box2D/Box2D/Collision;
    Box2D/Box2D/Box2D/Common;
    Box2D/Box2D/Box2D/Dynamics;
    Box2D/Box2D/Box2D/Rope;
")

set(BOX2D_INCLUDED_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/Box2D/Box2D/Build/gmake/bin/Debug/)
set(BOX2D_INCLUDED_LIBRARY ${CMAKE_SOURCE_DIR}/Box2D/Box2D/Build/gmake/bin/Debug/libBox2D.a)

include_directories (${GLEW_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR} ${BOX2D_INCLUDED_INCLUDE_DIR} ${GLM_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} lib/MultiNEAT/src)
target_link_libraries (evsim ${GLEW_LIBRARIES} glfw ${OPENGL_gl_LIBRARY} ${BOX2D_INCLUDED_LIBRARY} MultiNEAT)




