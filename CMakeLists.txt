cmake_minimum_required (VERSION 2.6)
project (evsim)

add_executable (evsim evsim.cc gfx_program.cc)
add_definitions (-DUSE_BOOST_RANDOM)
set_property (TARGET evsim PROPERTY CXX_STANDARD 17)

find_package (glfw3 REQUIRED)
find_package (GLEW REQUIRED)
find_package (OpenGL REQUIRED)
find_package (Box2D REQUIRED)
find_package (glm REQUIRED)
find_package (Boost REQUIRED)

include_directories (${GLEW_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR} ${BOX2D_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} MultiNEAT/include)
target_link_libraries (evsim ${GLEW_LIBRARIES} glfw ${OPENGL_gl_LIBRARY} ${BOX2D_LIBRARIES} ${CMAKE_SOURCE_DIR}/MultiNEAT/libMultiNEAT.a)
