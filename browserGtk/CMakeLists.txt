install (DIRECTORY "${PROJECT_SOURCE_DIR}/browser/web"   DESTINATION . PATTERN "Thumbs.db" EXCLUDE PATTERN "search*.htm" EXCLUDE PATTERN "*.js" EXCLUDE PATTERN "*.css" EXCLUDE)


file (GLOB GENERAL RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    [^.]*.c
    [^.]*.h
    )

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
    "../filesearch/env.h"
    "../filesearch/sharelib.h"
    "../filesearch/common.h"
    "../filesearch/common.c"
    "../filesearch/mac_port.h"
    "../filesearch/mac_port.c"
    "../browser/history.c"
    "../browser/history.h"
    )


find_package(PkgConfig)
pkg_check_modules(WEBKIT REQUIRED "webkit-1.0 >= 1.0")
add_definitions(${WEBKIT_CFLAGS})
include_directories(${WEBKIT_INCLUDE_DIRS})
link_directories(${WEBKIT_LIBRARY_DIRS})

add_executable (peasrch ${SOURCES})
target_link_libraries (peasrch "pthread")
target_link_libraries(peasrch ${WEBKIT_LIBRARIES})

#install (TARGETS peasrch BUNDLE DESTINATION .)

