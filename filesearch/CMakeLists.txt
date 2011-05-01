
include_directories ("${CMAKE_CURRENT_SOURCE_DIR}/sparsehash")
include_directories ("${CMAKE_CURRENT_SOURCE_DIR}/sparsehash/windows")

file (GLOB GENERAL RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    bzip2/[^.]*.c
    bzip2/[^.]*.h
    [^.]*.c
    [^.]*.cpp
    [^.]*.h
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
    )


# add the executable
add_library (GIGASO SHARED ${SOURCES} )
target_link_libraries (GIGASO "${PROJECT_SOURCE_DIR}/libs/libtcmalloc_minimal.lib")
set_target_properties(GIGASO PROPERTIES LINK_FLAGS "/include:__tcmalloc") 
set_target_properties(GIGASO PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"") 

add_executable ( "servernp" "serverNP.C" )
add_executable ( "clientnp" "clientNP.C" )


install (TARGETS GIGASO  DESTINATION .)
install (FILES "${PROJECT_SOURCE_DIR}/libs/libtcmalloc_minimal.dll"   DESTINATION .)
install (FILES "${PROJECT_SOURCE_DIR}/libs/hz.lib"   DESTINATION .)


