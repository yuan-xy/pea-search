set(BREAKPAD_DIR ${PROJECT_SOURCE_DIR}/../google-breakpad-read-only/src)

include_directories ("${CMAKE_CURRENT_SOURCE_DIR}/sparsehash")
include_directories ("${CMAKE_CURRENT_SOURCE_DIR}/sparsehash/windows")
include_directories ("${BREAKPAD_DIR}")


file (GLOB GENERAL RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
	bzip2/[^.]*.c
	bzip2/[^.]*.h
	[^.]*.cpp
	[^.]*.h
	chinese.c
	drive.c
	drive_change.c
	fat.c
	fs_common.c
	global.c
	history.c
	main.c
	ntfs.c
	search.c
	serverNP.C
	str_match.c
	suffix.c
	util.c
	write.c
	common.c
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
    win_service.c
    )


# add the executable
add_executable (gigaso ${SOURCES} )
target_link_libraries (gigaso "${PROJECT_SOURCE_DIR}/libs/libtcmalloc_minimal.lib")
target_link_libraries (gigaso "${BREAKPAD_DIR}/client/windows/Release/lib/exception_handler.lib")
target_link_libraries (gigaso "${BREAKPAD_DIR}/client/windows/Release/lib/crash_generation_client.lib")
target_link_libraries (gigaso "${BREAKPAD_DIR}/client/windows/Release/lib/common.lib")

set_target_properties(gigaso PROPERTIES LINK_FLAGS "/include:__tcmalloc") 
set_target_properties(gigaso PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"") 

install (TARGETS gigaso  DESTINATION .)
install (FILES "${PROJECT_SOURCE_DIR}/libs/libtcmalloc_minimal.dll"   DESTINATION .)
install (FILES "${PROJECT_SOURCE_DIR}/libs/hz.lib"   DESTINATION .)



SET( SOURCES
    ${GENERAL}
    ${GENERATED}
    test_np_server.c
    )

add_executable (test_np_server ${SOURCES} )
target_link_libraries (test_np_server "${PROJECT_SOURCE_DIR}/libs/libtcmalloc_minimal.lib")
target_link_libraries (test_np_server "${BREAKPAD_DIR}/client/windows/Release/lib/exception_handler.lib")
target_link_libraries (test_np_server "${BREAKPAD_DIR}/client/windows/Release/lib/crash_generation_client.lib")
target_link_libraries (test_np_server "${BREAKPAD_DIR}/client/windows/Release/lib/common.lib")

set_target_properties(test_np_server PROPERTIES LINK_FLAGS "/include:__tcmalloc") 
set_target_properties(test_np_server PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"") 
install (TARGETS test_np_server  DESTINATION .)

add_executable(test_np_client test_np_client.c)
set_target_properties(test_np_client PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='asInvoker' uiAccess='false'\"") 
install (TARGETS test_np_client DESTINATION .)


SET( SOURCES
    ${GENERAL}
    ${GENERATED}
    test_main.c
    )

add_executable (test_main ${SOURCES} )
target_link_libraries (test_main "${PROJECT_SOURCE_DIR}/libs/libtcmalloc_minimal.lib")
target_link_libraries (test_main "${BREAKPAD_DIR}/client/windows/Release/lib/exception_handler.lib")
target_link_libraries (test_main "${BREAKPAD_DIR}/client/windows/Release/lib/crash_generation_client.lib")
target_link_libraries (test_main "${BREAKPAD_DIR}/client/windows/Release/lib/common.lib")

set_target_properties(test_main PROPERTIES LINK_FLAGS "/include:__tcmalloc") 
set_target_properties(test_main PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"") 
install (TARGETS test_main  DESTINATION .)
