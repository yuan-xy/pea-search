SET( SOURCES
    "webform.h"
    "webform.cpp"
    "search.cpp"
    "../filesearch/common.h"
    "../filesearch/common.c"
    )
   
add_executable (search ${SOURCES} )
target_link_libraries (search "Shlwapi.lib")
set_target_properties(search PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /MANIFESTUAC:\"level='asInvoker' uiAccess='false'\"") 

install (TARGETS search DESTINATION .)
install (DIRECTORY "${PROJECT_SOURCE_DIR}/browser/web"   DESTINATION .)



set(CEF_DIR ${PROJECT_SOURCE_DIR}/3rd/cef_binary_r78_VS2005)
set(CEF_DLL_DIR ${CEF_DIR}/build/${CMAKE_BUILD_TYPE})

link_directories("${CEF_DIR}/lib/${CMAKE_BUILD_TYPE}")
add_executable (search_cef "search_cef.cpp" )
target_link_libraries (search_cef "libcef.lib libcef_dll_wrapper.lib")
set_target_properties(search_cef PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /MANIFESTUAC:\"level='asInvoker' uiAccess='false'\"") 

install (TARGETS search_cef DESTINATION .)
install (FILES "${CEF_DLL_DIR}/avcodec-52.dll" DESTINATION .)
install (FILES "${CEF_DLL_DIR}/avformat-52.dll" DESTINATION .)
install (FILES "${CEF_DLL_DIR}/avutil-50.dll" DESTINATION .)
install (FILES "${CEF_DLL_DIR}/icudt42.dll" DESTINATION .)
install (FILES "${CEF_DLL_DIR}/libcef.dll" DESTINATION .)
