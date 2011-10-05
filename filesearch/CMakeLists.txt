
include_directories ("${CMAKE_CURRENT_SOURCE_DIR}/sparsehash")
include_directories ("${CMAKE_CURRENT_SOURCE_DIR}/sparsehash/windows")
include_directories ("${BREAKPAD_DIR}")


file (GLOB GENERAL RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
	bzip2/[^.]*.c
	bzip2/[^.]*.h
	[^.]*.h
	chinese.c
	fs_common.c
	global.c
	history.c
	main.c
	search.c
	server.c
	serverNP.c
	str_match.c
	suffix.c
	util.c
	write.c
	common.c
	md5c.c
	chinese.cpp
	exception_dump.cpp
	fs_common2.cpp

	IF( WIN32 )
	drive.c
	drive_change.c
	fat.c	
	ntfs.c
	desktop.cpp
	desktop_exec.cpp
	ntfs.cpp
	ELSE( WIN32 )
	    message(STATUS "UNIX")
	ENDIF()	

	
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

add_library (file_search STATIC ${SOURCES} )
#set_target_properties(file_search PROPERTIES LINK_FLAGS "/include:__tcmalloc") 
target_link_libraries (file_search "${3RD_LIB_PATH}/libtcmalloc_minimal.lib")
target_link_libraries (file_search "${3RD_LIB_PATH}/exception_handler.lib")
target_link_libraries (file_search "${3RD_LIB_PATH}/crash_generation_client.lib")
target_link_libraries (file_search "${3RD_LIB_PATH}/common.lib")
target_link_libraries (file_search "${3RD_LIB_PATH}/crash_report_sender.lib")
target_link_libraries (file_search "Wininet.lib Urlmon.lib Shlwapi.lib")

add_executable (peadaemo "win_service.c" )
target_link_libraries (peadaemo "file_search")
set_target_properties(peadaemo PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"") 
install (TARGETS peadaemo  DESTINATION .)
install (FILES "${PROJECT_SOURCE_DIR}/libs/hz.lib"   DESTINATION .)



add_executable (test_np_server "test_np_server.c" )
target_link_libraries (test_np_server "file_search")
set_target_properties(test_np_server PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"") 
#install (TARGETS test_np_server  DESTINATION .)

add_executable(test_np_client test_np_client.c)
set_target_properties(test_np_client PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='asInvoker' uiAccess='false'\"") 
#install (TARGETS test_np_client DESTINATION .)



add_executable (test_main "test_main.c" )
target_link_libraries (test_main "file_search")
set_target_properties(test_main PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"") 
#install (TARGETS test_main  DESTINATION .)


add_executable (ctest_find_path "ctest_find_path.c" )
target_link_libraries (ctest_find_path "file_search")
ADD_TEST(find_path ${CMAKE_CURRENT_BINARY_DIR}/ctest_find_path)

add_executable (pealn "hotkey_listener.c" "common.c")
set_target_properties(pealn PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
install (TARGETS pealn  DESTINATION .)

add_executable (peadeskg "user_desk_gen.c" )
target_link_libraries (peadeskg "file_search")
set_target_properties(peadeskg PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"") 
install (TARGETS peadeskg  DESTINATION .)
