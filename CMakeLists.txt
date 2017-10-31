cmake_minimum_required (VERSION 2.6)
project (asd)

add_executable (ogl ogl.cc gfx_program.cc)

set_property (TARGET ogl PROPERTY CXX_STANDARD 17)

find_package (glfw3 REQUIRED)
find_package (GLEW REQUIRED)
find_package (OpenGL REQUIRED)
find_package (Box2D REQUIRED)

include_directories (${GLEW_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR} ${BOX2D_INCLUDE_DIRS})
target_link_libraries (ogl ${GLEW_LIBRARIES} glfw ${OPENGL_gl_LIBRARY} ${BOX2D_LIBRARIES})
