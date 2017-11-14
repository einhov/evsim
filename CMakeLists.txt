cmake_minimum_required (VERSION 2.6)
project (asd)

set (CMAKE_VERBOSE_MAKEFILE ON)
add_executable (ogl ogl.cc gfx_program.cc fuzzy.cc fuzzy.h)

set_property (TARGET ogl PROPERTY CXX_STANDARD 17)

find_package (glfw3 REQUIRED)
find_package (GLEW REQUIRED)
find_package (OpenGL REQUIRED)
find_package (Box2D REQUIRED)

set(FL_SYSTEM_LIBRARY FALSE) #Finds the library installed in the system
set(FL_LIBRARY_DIR)

set(FL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/../fuzzylite/fuzzylite)
set(FL_LIBRARY_DIR ${PROJECT_SOURCE_DIR}/../fuzzylite/fuzzylite/build/bin)

add_definitions(-DFL_CPP11)
find_library (FuzzyLiteLibrary fuzzylite HINTS ${FL_LIBRARY_DIR})

include_directories (${GLEW_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR} ${BOX2D_INCLUDE_DIRS} ${FL_INCLUDE_DIR})
target_link_libraries (ogl ${GLEW_LIBRARIES} glfw ${OPENGL_gl_LIBRARY} ${BOX2D_LIBRARIES} ${FL_LIBRARY_DIR}/libfuzzylite-static.a)
