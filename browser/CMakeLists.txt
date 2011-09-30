include_directories ("${BREAKPAD_DIR}")

install (DIRECTORY "${PROJECT_SOURCE_DIR}/browser/web"   DESTINATION . PATTERN "Thumbs.db" EXCLUDE PATTERN "search.htm" EXCLUDE PATTERN "*.js" EXCLUDE PATTERN "*.css" EXCLUDE)

set(CEF_DIR ${PROJECT_SOURCE_DIR}/3rd/cef_binary)
set(CEF_DLL_DIR ${CEF_DIR}/build/${CMAKE_BUILD_TYPE})

link_directories("${CEF_DIR}/lib/${CMAKE_BUILD_TYPE}")

file (GLOB GENERAL RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    explorer++/[^.]*.cpp
    explorer++/[^.]*.h
    [^.]*.cpp
    [^.]*.h
    "history.c"
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
    "peasrch.rc"
    "../filesearch/env.h"
    "../filesearch/sharelib.h"
    "../filesearch/common.h"
    "../filesearch/common.c"
    "../filesearch/desktop_exec.h"
    "../filesearch/desktop_exec.cpp"
    "../filesearch/exception_dump.cpp"
    )

add_executable (peasrch  ${SOURCES})
target_link_libraries (peasrch "libcef.lib libcef_dll_wrapper.lib")
target_link_libraries (peasrch "${3RD_LIB_PATH}/exception_handler.lib")
target_link_libraries (peasrch "${3RD_LIB_PATH}/crash_generation_client.lib")
target_link_libraries (peasrch "${3RD_LIB_PATH}/common.lib")
target_link_libraries (peasrch "${3RD_LIB_PATH}/crash_report_sender.lib")
target_link_libraries (peasrch "Wininet.lib")

set_target_properties(peasrch PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /MANIFESTUAC:\"level='asInvoker' uiAccess='false'\"") 

install (TARGETS peasrch DESTINATION .)
#install (FILES "${CEF_DLL_DIR}/avcodec-52.dll" DESTINATION .)
#install (FILES "${CEF_DLL_DIR}/avformat-52.dll" DESTINATION .)
#install (FILES "${CEF_DLL_DIR}/avutil-50.dll" DESTINATION .)
install (FILES "${CEF_DLL_DIR}/icudt.dll" DESTINATION .)
install (FILES "${CEF_DLL_DIR}/libcef.dll" DESTINATION .)


SET( SOURCES1
    history.h
    history.c
    test_history.c
    win_misc.h
    win_misc.cpp

    )

add_executable(test_history ${SOURCES1} )
set_target_properties(test_history PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='asInvoker' uiAccess='false'\"") 

SET( SOURCES2
    bitmap.h
    bitmap.cpp
    win_icon.h
    win_icon.cpp
    test_win_icon.c
    )

add_executable(test_win_icon ${SOURCES2} )
set_target_properties(test_win_icon PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='asInvoker' uiAccess='false'\"") 
