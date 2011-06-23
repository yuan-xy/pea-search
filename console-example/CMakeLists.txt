add_executable(testcons test.c)

set_target_properties(testcons PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='asInvoker' uiAccess='false'\"") 

install (TARGETS testcons DESTINATION .)


add_executable(sctrl win_service_ctrl.c)

set_target_properties(sctrl PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"") 

# install (TARGETS sctrl DESTINATION .)
