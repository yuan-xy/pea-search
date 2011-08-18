#include "env.h"

static BOOL HasAnOtherProcess(){
        HANDLE hMutex = CreateMutex( NULL, FALSE, L"GigasoHotkeyListener" );
        if ( GetLastError() == ERROR_ALREADY_EXISTS ){
                CloseHandle( hMutex );
                return 1;
        }
        return 0;
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
    MSG msg = {0};
	if(HasAnOtherProcess()) return 1;
    if (!RegisterHotKey(NULL,1, 0, VK_PAUSE)) {
        return 1;
    }
    while(GetMessage(&msg, NULL, 0, 0) != 0){
        if (msg.message == WM_HOTKEY){
			HWND wnd = FindWindow(SearchWindowClass,SearchWindowTitle);
			if(wnd==NULL){
				HINSTANCE  hi = ShellExecute(NULL, L"open", L"search.exe",NULL,NULL,SW_SHOW); 
				DWORD ret = GetLastError();
			}else{
				ShowWindow(wnd, SW_SHOW);  // Make the window visible if it was hidden
				ShowWindow(wnd, SW_RESTORE);  // Next, restore it if it was minimized
				SetForegroundWindow(wnd);  // Finally, activate the window
				//SwitchToThisWindow(wnd,0);
			}
		}
    }
	return 0;
}
