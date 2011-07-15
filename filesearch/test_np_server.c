#include "env.h"
#include "util.h"
#include "serverNP.h"
#include "main.h"

static DWORD WINAPI  Hotkey(PVOID pParam){
    MSG msg = {0};
    if (!RegisterHotKey(NULL,1, MOD_ALT, 0x42)) {	//0x42 is 'b'
        return 1;
    }
    wprintf(L"Hotkey 'ALT+b' registered, using MOD_NOREPEAT flag\n");
    while(GetMessage(&msg, NULL, 0, 0) != 0){
        if (msg.message == WM_HOTKEY){
            wprintf(L"WM_HOTKEY received\n");
            WinExec("search.exe", SW_SHOW);
        }
    }
	return 0;
}


int main(){
	if (!SetConsoleCtrlHandler(shutdown_handle, TRUE)) {
		WIN_ERROR;
		return 3;
	}
	gigaso_init();
	if(start_named_pipe()){
		CreateThread(NULL,0,Hotkey,NULL,0,0);
		wait_stop_named_pipe();
	}
	gigaso_destory();
	ExitProcess(0);
	return 0;
}
