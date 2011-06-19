add_executable(Test test.c)

set_target_properties(Test PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"") 
