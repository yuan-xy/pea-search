cmake_minimum_required (VERSION 2.6)
set (CMAKE_BACKWARDS_COMPATIBILITY 2.6)
set (CMAKE_VERBOSE_MAKEFILE on)

project (GIGASO)

set (GIGASO_VERSION_MAJOR 1)
set (GIGASO_VERSION_MINOR 0)

configure_file (
  "${PROJECT_SOURCE_DIR}/GIGASOConfig.h.in"
  "${PROJECT_BINARY_DIR}/GIGASOConfig.h"
  )

# add the binary tree to the search path for include files
# so that we will find GIGASOConfig.h
include_directories ("${PROJECT_BINARY_DIR}")


include_directories ("${PROJECT_SOURCE_DIR}/filesearch")
add_subdirectory (filesearch)

include_directories ("${PROJECT_SOURCE_DIR}/console-example")
add_subdirectory (console-example)