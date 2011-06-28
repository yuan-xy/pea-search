SET( SOURCES
    "webform.h"
    "webform.cpp"
    "search.cpp"
    "../filesearch/common.h"
    "../filesearch/common.c"
    )
   
add_executable (search ${SOURCES} )

set_target_properties(search PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /MANIFESTUAC:\"level='asInvoker' uiAccess='false'\"") 

install (TARGETS search DESTINATION .)
install (DIRECTORY "${PROJECT_SOURCE_DIR}/browser/web"   DESTINATION .)
