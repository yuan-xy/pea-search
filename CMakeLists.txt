cmake_minimum_required (VERSION 2.8)
set (CMAKE_BACKWARDS_COMPATIBILITY 2.8)
set (CMAKE_VERBOSE_MAKEFILE on)

#	-DCMAKE_BUILD_TYPE=Release ..
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release  CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
	message(STATUS "default build type: ${CMAKE_BUILD_TYPE}")
ELSE(NOT CMAKE_BUILD_TYPE)
	message(STATUS "define build type: ${CMAKE_BUILD_TYPE}")
ENDIF(NOT CMAKE_BUILD_TYPE)

set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob2 /D NDEBUG /MT /Zi /D UNICODE /D _UNICODE ")
SET(CMAKE_C_FLAGS_RELEASE "/O2 /Ob2 /D NDEBUG /MT /Zi /D UNICODE /D _UNICODE ")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /D UNICODE /D _UNICODE ")
SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd /D UNICODE /D _UNICODE ")
message(STATUS ${CMAKE_CXX_FLAGS_RELEASE})

ENABLE_TESTING()

project (GIGASO)

set (GIGASO_VERSION_MAJOR 1)
set (GIGASO_VERSION_MINOR 0)
set (GIGASO_VERSION_BUILD 14)


message(STATUS "size of pointer: ${CMAKE_SIZEOF_VOID_P}  ")
IF( ${CMAKE_SIZEOF_VOID_P} EQUAL  4 )
    message(STATUS "32 bits")
ELSE(${CMAKE_SIZEOF_VOID_P} EQUAL "4")
    set( HAVE_64_BIT 1 )
    set( CMAKE_SYSTEM_PROCESSOR "x64" ) 
    message(STATUS "64 bits")
ENDIF()
message(STATUS "size of pointer: ${CMAKE_SYSTEM_PROCESSOR}  ")

SET(3RD_LIB_PATH "${PROJECT_SOURCE_DIR}/libs/${CMAKE_SYSTEM_PROCESSOR}/${CMAKE_BUILD_TYPE}")
message(STATUS ${3RD_LIB_PATH})

set(BREAKPAD_DIR ${PROJECT_SOURCE_DIR}/3rd/google-breakpad-read-only/src)

configure_file (
  "${PROJECT_SOURCE_DIR}/GIGASOConfig.h.in"
  "${PROJECT_BINARY_DIR}/GIGASOConfig.h"
  )

EXEC_PROGRAM("ruby ${PROJECT_SOURCE_DIR}/html_embed.rb" "${PROJECT_BINARY_DIR}")

# add the binary tree to the search path for include files
# so that we will find GIGASOConfig.h
include_directories ("${PROJECT_BINARY_DIR}")


include_directories ("${PROJECT_SOURCE_DIR}/filesearch")
add_subdirectory (filesearch)

include_directories ("${PROJECT_SOURCE_DIR}/console-example")
add_subdirectory (console-example)

include_directories ("${PROJECT_SOURCE_DIR}/browser")
add_subdirectory (browser)

install (FILES "${PROJECT_SOURCE_DIR}/cmake/gigaso.inf"   DESTINATION .)


set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(CPACK_PACKAGE_NAME "PeaSearch")
set(CPACK_PACKAGE_DESCRIPTION "Do the file search")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A longer description about our PeaSearch app.")
set(CPACK_PACKAGE_CONTACT "yuanxinyu@peasrch.com")
set(CPACK_PACKAGE_VENDOR "PeaSrch.com")
set(CPACK_RESOURCE_FILE_LICENSE  "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${GIGASO_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${GIGASO_VERSION_MINOR}.${GIGASO_VERSION_BUILD}")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${GIGASO_VERSION_MINOR}.${GIGASO_VERSION_BUILD}-${CMAKE_SYSTEM_PROCESSOR}") 
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Wandouss")

IF(WIN32 AND NOT UNIX)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backslashes.
  SET(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/browser\\\\web\\\\logo.ico")
  SET(CPACK_NSIS_DISPLAY_NAME "Íã¶¹ËÑË÷£¨wandouss.com£©")
  SET(CPACK_NSIS_PACKAGE_NAME "Íã¶¹ËÑË÷")
  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.wandouss.com")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.wandouss.com")
  SET(CPACK_NSIS_CONTACT "yuanxinyu@peasrch.com")
ENDIF(WIN32 AND NOT UNIX)

INCLUDE(CPack)

