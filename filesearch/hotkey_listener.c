#include "env.h"
#include "common.h"

static BOOL ExistListener(){
        HANDLE hMutex = CreateMutex( NULL, FALSE, L"GigasoHotkeyListener" );
        if ( GetLastError() == ERROR_ALREADY_EXISTS ){
                CloseHandle( hMutex );
                return 1;
        }
        return 0;
}

static BOOL register_hotkey(){
	BOOL flag;
	int key = get_hotkey();
	switch(key){
		case 1: 	flag = RegisterHotKey(NULL,1, MOD_ALT, VK_PAUSE);break;
		case 2: 	flag = RegisterHotKey(NULL,1, 0, VK_F7);break;
		case 3: 	flag = RegisterHotKey(NULL,1, MOD_ALT, VK_F7);break;
		case 4: 	flag = RegisterHotKey(NULL,1, 0, VK_F8);break;
		case 5: 	flag = RegisterHotKey(NULL,1, MOD_ALT, VK_F8);break;
		case 6: 	flag = RegisterHotKey(NULL,1, 0, VK_F9);break;
		case 7: 	flag = RegisterHotKey(NULL,1, MOD_ALT, VK_F9);break;
		default: 	flag = RegisterHotKey(NULL,1, 0, VK_PAUSE);break;
	}
	return flag;
}

static BOOL re_reg_hotkey(){
	UnregisterHotKey(NULL,1);
	return register_hotkey();
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
		if (message == WM_SET_HOTKEY){
			re_reg_hotkey();
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
}

ATOM MyRegisterClass(HINSTANCE hInstance){
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= ListenerWindowClass;
	wcex.hIconSm		= NULL;
	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow){
	HWND hMainWin = CreateWindow(ListenerWindowClass, L"",
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, 0, 100,
		100, NULL, NULL, hInstance, NULL);
	return hMainWin!=NULL;
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
    MSG msg = {0};
	if(ExistListener()) return 1;
	setPWD(NULL);
    if (!register_hotkey()) return 1;
	MyRegisterClass(hInstance);
	if (!InitInstance (hInstance, iCmdShow)) return 1;
	WinExec("peadeskg.exe",SW_HIDE);
	LoadLibrary(L"libcef.dll");
	LoadLibrary(L"icudt.dll");
    while(GetMessage(&msg, NULL, 0, 0) != 0){
		if (msg.message == WM_HOTKEY){
			HWND wnd = FindWindow(SearchWindowClass,SearchWindowTitle);
			if(wnd==NULL){
				HINSTANCE  hi = ShellExecute(NULL, L"open", L"peasrch.exe",NULL,NULL,SW_SHOW); 
				DWORD ret = GetLastError();
			}else{
				ShowWindow(wnd, SW_SHOW);  // Make the window visible if it was hidden
				ShowWindow(wnd, SW_RESTORE);  // Next, restore it if it was minimized
				SetForegroundWindow(wnd);  // Finally, activate the window
				//SwitchToThisWindow(wnd,0);
			}
		}else{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
    }
	return 0;
}
