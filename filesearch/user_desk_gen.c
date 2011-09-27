#include "env.h"
#include "desktop.h"

int main(){
	BOOL ret = exec_desktop(L"\\我的电脑\\控制面板\\键盘");
	scan_desktop();
	return 0;
}
