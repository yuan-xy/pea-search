cmake_minimum_required (VERSION 2.6)
set (CMAKE_BACKWARDS_COMPATIBILITY 2.6)
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
set (GIGASO_VERSION_BUILD 7)

set(BREAKPAD_DIR ${PROJECT_SOURCE_DIR}/3rd/google-breakpad-read-only/src)

configure_file (
  "${PROJECT_SOURCE_DIR}/GIGASOConfig.h.in"
  "${PROJECT_BINARY_DIR}/GIGASOConfig.h"
  )

configure_file (
  "${PROJECT_BINARY_DIR}/GIGASOConfig.h"
  "${PROJECT_SOURCE_DIR}/PluginGigaso/GIGASOConfig.h"
  )

configure_file (
  "${PROJECT_SOURCE_DIR}/filesearch/env.h"
  "${PROJECT_SOURCE_DIR}/PluginGigaso/env.h"
  )

configure_file (
  "${PROJECT_SOURCE_DIR}/filesearch/sharelib.h"
  "${PROJECT_SOURCE_DIR}/PluginGigaso/sharelib.h"
  )

configure_file (
  "${PROJECT_SOURCE_DIR}/filesearch/common.h"
  "${PROJECT_SOURCE_DIR}/PluginGigaso/common.h"
  )

configure_file (
  "${PROJECT_SOURCE_DIR}/filesearch/common.c"
  "${PROJECT_SOURCE_DIR}/PluginGigaso/common.c"
  )


# add the binary tree to the search path for include files
# so that we will find GIGASOConfig.h
include_directories ("${PROJECT_BINARY_DIR}")


include_directories ("${PROJECT_SOURCE_DIR}/filesearch")
add_subdirectory (filesearch)

include_directories ("${PROJECT_SOURCE_DIR}/console-example")
add_subdirectory (console-example)

include_directories ("${PROJECT_SOURCE_DIR}/browser")
add_subdirectory (browser)

install (FILES "${PROJECT_SOURCE_DIR}/../firebreath-1.4/release/bin/PluginGigaso/npPluginGigaso.dll"   DESTINATION .)
install (FILES "${PROJECT_SOURCE_DIR}/cmake/gigaso.inf"   DESTINATION .)


set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(CPACK_PACKAGE_NAME "Gigaso")
set(CPACK_PACKAGE_DESCRIPTION "Do the file search")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A longer description about our Gigaso app.")
set(CPACK_PACKAGE_CONTACT "yuanxinyu@gigaso.com")
set(CPACK_PACKAGE_VENDOR "Gigaso.com")
set(CPACK_RESOURCE_FILE_LICENSE  "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${GIGASO_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${GIGASO_VERSION_MINOR}.${GIGASO_VERSION_BUILD}")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${GIGASO_VERSION_MINOR}-${CMAKE_SYSTEM_PROCESSOR}") 
set(CPACK_PACKAGE_INSTALL_DIRECTORY "gigaso")

IF(WIN32 AND NOT UNIX)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backslashes.
  SET(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/browser\\\\web\\\\images\\\\find.ico")
  SET(CPACK_NSIS_DISPLAY_NAME "记佳文件搜索（Gigaso.com）")
  SET(CPACK_NSIS_PACKAGE_NAME "记佳文件搜索")
  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.gigaso.com")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.gigaso.com")
  SET(CPACK_NSIS_CONTACT "yuanxinyu@gigaso.com")
ENDIF(WIN32 AND NOT UNIX)

INCLUDE(CPack)



