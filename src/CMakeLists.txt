cmake_minimum_required(VERSION 2.8)
project(p4)

# this script looks for dependencies like SFML
include(cmake/CMakeLists.txt)

set( CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/.. )

set(CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS}")

include_directories(
	${PROJECT_SOURCE_DIR}
	${GLEW_INCLUDE_DIRS}
)

# the scene description and parsing code
add_subdirectory(scene)

# a library of rendering code
add_subdirectory(renderer)

# the main application
add_subdirectory(application)
