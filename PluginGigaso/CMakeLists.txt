#/**********************************************************\ 
# 
# Auto-generated CMakeLists.txt for the Plugin Gigaso project
#
#\**********************************************************/

# Written to work with cmake 2.6
cmake_minimum_required (VERSION 2.6)
set (CMAKE_BACKWARDS_COMPATIBILITY 2.6)

include(${CMAKE_DIR}/common.cmake)

set (PROJNAME ${PLUGIN_NAME})

browserplugin_project(${PROJNAME})

file (GLOB GENERAL RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    [^.]*.cpp
    [^.]*.h
    [^.]*.cmake
    )

# Generated files are stored in ${GENERATED} by the project configuration
SET_SOURCE_FILES_PROPERTIES(
    ${GENERATED}
    PROPERTIES
        GENERATED 1
    )

SOURCE_GROUP(Generated FILES
    ${GENERATED}
    )

SET( SOURCES
    ${GENERAL}
    ${GENERATED}
    history.c
    )

# This will include Win/projectDef.cmake, Linux/projectDef.cmake, etc
include_platform()

SET( SOURCES1
    history.h
    history.c
    test_history.c
    )

add_executable(test_history ${SOURCES1} )
set_target_properties(test_history PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='asInvoker' uiAccess='false'\"") 

SET( SOURCES2
    win_icon.h
    win_icon.cpp
    test_win_icon.c
    )

add_executable(test_win_icon ${SOURCES2} )
set_target_properties(test_win_icon PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='asInvoker' uiAccess='false'\"") 
