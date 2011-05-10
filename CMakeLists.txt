cmake_minimum_required (VERSION 2.6)
set (CMAKE_BACKWARDS_COMPATIBILITY 2.6)
set (CMAKE_VERBOSE_MAKEFILE off)

#	-DCMAKE_BUILD_TYPE=Release ..
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release  CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
	message(STATUS "default build type: ${CMAKE_BUILD_TYPE}")
ELSE(NOT CMAKE_BUILD_TYPE)
	message(STATUS "define build type: ${CMAKE_BUILD_TYPE}")
ENDIF(NOT CMAKE_BUILD_TYPE)

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


INCLUDE(CPack)