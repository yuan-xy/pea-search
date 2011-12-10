install (DIRECTORY "${PROJECT_SOURCE_DIR}/browser/web"   DESTINATION . PATTERN "Thumbs.db" EXCLUDE PATTERN "search*.htm" EXCLUDE PATTERN "*.js" EXCLUDE PATTERN "*.css" EXCLUDE)


file (GLOB GENERAL RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    [^.]*.m
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
    "../filesearch/history.h"
    "../filesearch/history.c"
    "../filesearch/mac_port.h"
    "../filesearch/mac_port.c"
    )

set_source_files_properties(${GENERAL} PROPERTIES COMPILE_FLAGS "-x objective-c+")
FIND_LIBRARY(COCOA_LIBRARY COCOA)
MARK_AS_ADVANCED(COCOA_LIBRARY)

#add_executable (peasrch MACOSX_BUNDLE ${SOURCES})
#target_link_libraries (peasrch "Webkit" ${COCOA_LIBRARY})
#install (TARGETS peasrch BUNDLE DESTINATION .)

