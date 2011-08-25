include_directories ("${BREAKPAD_DIR}")

SET( SOURCES
    "webform.h"
    "webform.cpp"
    "search_ie.cpp"
    "../filesearch/common.h"
    "../filesearch/common.c"
    )
   
add_executable (search_ie ${SOURCES} )
target_link_libraries (search_ie "Shlwapi.lib")
set_target_properties(search_ie PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /MANIFESTUAC:\"level='asInvoker' uiAccess='false'\"") 

#install (TARGETS search_ie DESTINATION .)
install (DIRECTORY "${PROJECT_SOURCE_DIR}/browser/web"   DESTINATION . PATTERN "Thumbs.db" EXCLUDE)



set(CEF_DIR ${PROJECT_SOURCE_DIR}/3rd/cef_binary)
set(CEF_DLL_DIR ${CEF_DIR}/build/${CMAKE_BUILD_TYPE})

link_directories("${CEF_DIR}/lib/${CMAKE_BUILD_TYPE}")
SET( SOURCES
    "string_util.h"
    "string_util.cpp"
    "cef_util.h"
    "client_handler.h"
    "client_handler.cpp"
    "cef_scheme.h"
    "cef_scheme.cpp"
    "cef_js.h"
    "cef_js.cpp"
    "search_cef.cpp"
    "../filesearch/env.h"
    "../filesearch/common.h"
    "../filesearch/common.c"
    "../filesearch/exception_dump.cpp"
    "../PluginGigaso/bitmap.h"
    "../PluginGigaso/bitmap.cpp"
    "../PluginGigaso/win_icon.h"
    "../PluginGigaso/win_icon.cpp"
    )
add_executable (search  ${SOURCES})
target_link_libraries (search "libcef.lib libcef_dll_wrapper.lib")
target_link_libraries (search "${BREAKPAD_DIR}/client/windows/${CMAKE_BUILD_TYPE}/lib/exception_handler.lib")
target_link_libraries (search "${BREAKPAD_DIR}/client/windows/${CMAKE_BUILD_TYPE}/lib/crash_generation_client.lib")
target_link_libraries (search "${BREAKPAD_DIR}/client/windows/${CMAKE_BUILD_TYPE}/lib/common.lib")
target_link_libraries (search "${BREAKPAD_DIR}/client/windows/${CMAKE_BUILD_TYPE}/lib/crash_report_sender.lib")
target_link_libraries (search "Wininet.lib")

set_target_properties(search PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /MANIFESTUAC:\"level='asInvoker' uiAccess='false'\"") 

install (TARGETS search DESTINATION .)
#install (FILES "${CEF_DLL_DIR}/avcodec-52.dll" DESTINATION .)
#install (FILES "${CEF_DLL_DIR}/avformat-52.dll" DESTINATION .)
#install (FILES "${CEF_DLL_DIR}/avutil-50.dll" DESTINATION .)
install (FILES "${CEF_DLL_DIR}/icudt.dll" DESTINATION .)
install (FILES "${CEF_DLL_DIR}/libcef.dll" DESTINATION .)
