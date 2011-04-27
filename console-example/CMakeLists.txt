add_executable(Test test.c)

if (${CMAKE_GENERATOR} MATCHES "Visual Studio" )
	target_link_libraries (Test "${PROJECT_BINARY_DIR}/filesearch/Debug/GIGASO.lib")
else (${CMAKE_GENERATOR} MATCHES "Visual Studio" )
	target_link_libraries (Test "${PROJECT_BINARY_DIR}/filesearch/GIGASO.lib")
endif (${CMAKE_GENERATOR} MATCHES "Visual Studio" )


set_target_properties(Test PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"") 

install (TARGETS Test DESTINATION .)
