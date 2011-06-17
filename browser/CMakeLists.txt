SET( SOURCES
    "webform.h"
    "webform.cpp"
    "search.cpp"
    )
   
add_executable (Search ${SOURCES} )

set_target_properties(Search PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /MANIFESTUAC:\"level='asInvoker' uiAccess='false'\"") 

install (TARGETS Search DESTINATION .)
