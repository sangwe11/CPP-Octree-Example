cmake_minimum_required(VERSION 2.8)
project (Octree_Example)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")
set(CMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD 1)
set(CMAKE_INSTALL_PREFIX "${CMAKE_HOME_DIRECTORY}")

add_library(Octree STATIC Octree.h)

find_package(GLM REQUIRED)
target_link_libraries (Octree GLM::GLM)

target_sources(Octree PRIVATE AABB.h)
target_sources(Octree PRIVATE AABB.cpp)

target_sources(Octree PRIVATE Intersection.h)
target_sources(Octree PRIVATE Intersection.cpp)

target_sources(Octree PRIVATE Octree.cpp)

install(TARGETS Octree
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib/static)