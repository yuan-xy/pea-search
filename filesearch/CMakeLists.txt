

if (WIN32) 
 include_directories ("${PROJECT_SOURCE_DIR}/3rd/sparsehash-1.11/src-win")
 include_directories ("${PROJECT_SOURCE_DIR}/3rd/sparsehash-1.11/src-win/windows")
 include_directories ("${BREAKPAD_DIR}")
ELSE( WIN32 )
 include_directories ("${PROJECT_SOURCE_DIR}/3rd/sparsehash-1.11/src-mac")
endif (WIN32)




file (GLOB GENERAL RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
	[^.]*.h
	chinese.c
	drive.c
	fs_common.c
	global.c
	main.c
	search.c
	server.c
	str_match.c
	suffix.c
	util.c
	write.c
	common.c
	md5c.c
	chinese.cpp
	fs_common2.cpp	
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

if (WIN32) 
	SET(SOURCES  ${SOURCES}
		drive_change.c
		drive_win.c
		fs_common_win.c
		fat.c	
		ntfs.c
		desktop.cpp
		desktop_exec.cpp
		ntfs.cpp
		main_win.c
		download_win.c
		serverNP.c
		exception_dump.cpp
	   )
endif (WIN32)
	
if (UNIX) 
		#ADD_DEFINITIONS(-Wall)
		set(SOURCES ${SOURCES}
		posix_port.c
		posixfs.c
		main_mac.c
		download_mac.c
		error_mac.c
		serverUD.c
		exception_dump_mac.cpp
		unix_domain_client.c
	)
	if (APPLE) 
		set(SOURCES ${SOURCES}
			drive_mac.c
			macfs.c
		)
	else (APPLE)
		set(SOURCES ${SOURCES}
			linux_inotify.c
		)
	endif (APPLE)
endif(UNIX)



if (WIN32) 
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
target_link_libraries (test_np_client "file_search")
set_target_properties(test_np_client PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='asInvoker' uiAccess='false'\"") 
#install (TARGETS test_np_client DESTINATION .)



add_executable (test_main "test_main.c" )
target_link_libraries (test_main "file_search")
set_target_properties(test_main PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"") 
#install (TARGETS test_main  DESTINATION .)


add_executable (ctest_find_path "ctest_find_path.c" )
target_link_libraries (ctest_find_path "file_search")
ADD_TEST(find_path ${CMAKE_CURRENT_BINARY_DIR}/ctest_find_path)

add_executable (pealn "hotkey_listener.c" "common.c" "hotkey.c")
set_target_properties(pealn PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
install (TARGETS pealn  DESTINATION .)

add_executable (peadeskg "user_desk_gen.c" )
target_link_libraries (peadeskg "file_search")
set_target_properties(peadeskg PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='asInvoker' uiAccess='false'\"") 
install (TARGETS peadeskg  DESTINATION .)

endif (WIN32)


if (UNIX) 
	add_library (file_search STATIC ${SOURCES} )
	if (APPLE) 
		FIND_LIBRARY(CoreServices_LIB CoreServices)
		FIND_LIBRARY(CoreFoundation_LIB CoreFoundation)
		MARK_AS_ADVANCED (CoreServices_LIB
		                     CoreFoundation_LIB)
		SET(EXTRA_LIBS ${CoreServices_LIB} ${CoreFoundation_LIB})	
		target_link_libraries (file_search ${EXTRA_LIBS})
		target_link_libraries (file_search "${3RD_LIB_PATH}/libtcmalloc_minimal.a")
		#set_target_properties(file_search PROPERTIES LINK_FLAGS "-framework CoreServices -framework CoreFoundation") 
	else (APPLE) 
		target_link_libraries (file_search "pthread")			
	endif (APPLE)
	add_executable (test_main "test_main.c" )
	target_link_libraries (test_main "file_search")
	add_executable (test_server "test_server.c" )
	target_link_libraries (test_server "file_search")	
	add_executable (test_np_server "test_np_server.c" )
	target_link_libraries (test_np_server "file_search")
	
	add_executable(test_np_client test_np_client_mac.c)
	target_link_libraries (test_np_client "file_search")	
endif(UNIX)

add_executable(test_history "common.c" "history.c" "test_history.c")
ADD_TEST(test_history ${CMAKE_CURRENT_BINARY_DIR}/test_history)
