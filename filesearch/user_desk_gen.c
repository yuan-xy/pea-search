#include "env.h"
#include <stdio.h>
#include "desktop.h"
#include "desktop_exec.h"
#include "write.h"

static BOOL save_desktop_self(pFileEntry desktop){
	wchar_t fbuffer[MAX_PATH];
	DWORD size=MAX_PATH;
	if(GetUserName(fbuffer, &size)){
		save_desktop(fbuffer,desktop);
		return 1;
	}else{
		return 0;
	}
}

int main(){
	//save_desktop_self(scan_desktop());
	if(_waccess(ALL_DESKTOP,0)==-1){
		save_desktop(ALL_DESKTOP, scan_desktop());
		//exec_desktop(L"\\我的电脑\\控制面板\\键盘");
	}
	return 0;
}
