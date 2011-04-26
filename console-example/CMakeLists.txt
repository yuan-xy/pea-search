add_executable(Test test.c)
target_link_libraries (Test "${PROJECT_BINARY_DIR}/filesearch/Debug/GIGASO.lib")
set_target_properties(Test PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"") 

install (TARGETS Test DESTINATION bin)
