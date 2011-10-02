#include "../filesearch/env.h"
#include "../3rd/cef_binary/include/cef.h"
#include "../filesearch/common.h"
#include "cef_scheme.h"
#include "cef_js.h"
#include "cef_plugin.h"
#include "client_handler.h"
#include <sstream>
#include <string>
#include "exception_dump.h"
#include "resource.h"

#define MAX_LOADSTRING 100
#define MAX_URL_LENGTH  255
//#define TEST_SINGLE_THREADED_MESSAGE_LOOP

HINSTANCE hInst;								// current instance
HWND hMainWin;
CefRefPtr<ClientHandler> g_handler;

#if defined(OS_WIN)
// Add Common Controls to the application manifest because it's required to
// support the default tooltip implementation.
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);


static void trans_focus();

int APIENTRY wWinMain(HINSTANCE hInstance,
					  HINSTANCE hPrevInstance,
					  LPTSTR    lpCmdLine,
					  int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	breakpad_init();
	setPWD(NULL);

  CefSettings settings;

#ifdef NDEBUG
  settings.log_severity = LOGSEVERITY_ERROR;
#endif

#ifdef TEST_SINGLE_THREADED_MESSAGE_LOOP
  settings.multi_threaded_message_loop = false;
#else
  settings.multi_threaded_message_loop = true;
#endif

  CefInitialize(settings);
	InitSchemeTest();
	InitExtensionTest();
	InitPlugin();
	MyRegisterClass(hInstance);
	if (!InitInstance (hInstance, nCmdShow)) return FALSE;
	if(OleInitialize(NULL)!=S_OK) MessageBox(NULL,L"warn",L"ole init failed.",MB_OK);
	connect_named_pipe();
	SetForegroundWindow(hMainWin);
	SetFocus(hMainWin);
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)){
#ifdef TEST_SINGLE_THREADED_MESSAGE_LOOP
		// Allow the CEF to do its message loop processing.
		CefDoMessageLoopWork();
#endif
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	close_named_pipe();
	OleUninitialize();
	CefShutdown();
	return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance){
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon = LoadIcon (hInstance, MAKEINTRESOURCE (IDI_ICON1)) ;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= SearchWindowClass;
	wcex.hIconSm		= NULL;
	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow){
	hInst = hInstance;
	hMainWin = CreateWindow(SearchWindowClass, SearchWindowTitle,
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, 0, 1000,
		680, NULL, NULL, hInstance, NULL);
	if (!hMainWin) return FALSE;
	ShowWindow(hMainWin, nCmdShow);
	UpdateWindow(hMainWin);
	return TRUE;
}

static void trans_focus(){
	if(g_handler.get() && g_handler->GetBrowserHwnd()){
		PostMessage(g_handler->GetBrowserHwnd(), WM_SETFOCUS, NULL, NULL);
		exec_js_str(L"try{ $('#search').select();}catch(e){}");
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
	PAINTSTRUCT ps;
	HDC hdc;
	switch (message){
		case WM_CREATE:{
				g_handler = new ClientHandler();
				RECT rect;
				GetClientRect(hWnd, &rect);
				CefBrowserSettings settings;
				settings.plugins_disabled = false;
				settings.file_access_from_file_urls_allowed = true;
				settings.universal_access_from_file_urls_allowed = true;
				CefWindowInfo info;
				info.SetAsChild(hWnd, rect);
				wchar_t full_path[MAX_PATH];
				get_abs_path(L"web\\search.htm",full_path);
				if(_waccess(full_path,0)==0){
					CefBrowser::CreateBrowser(info,
						static_cast<CefRefPtr<CefClient>>(g_handler), full_path,settings);
				}else{
					CefBrowser::CreateBrowser(info,
						static_cast<CefRefPtr<CefClient>>(g_handler), L"about:blank",settings);
				}
			}
			return 0;
		case WM_ACTIVATE:
			if((short)wParam!=WA_INACTIVE){
				//SetForegroundWindow(hWnd);
				trans_focus();
			}
			return 0;
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			return 0;
		case WM_SETFOCUS:
			trans_focus();
			return 0;
		case WM_SIZE:
			if(g_handler.get() && g_handler->GetBrowserHwnd()){
				RECT rect;
				GetClientRect(hWnd, &rect);
				HDWP hdwp = BeginDeferWindowPos(1);
				hdwp = DeferWindowPos(hdwp, g_handler->GetBrowserHwnd(), NULL,
					rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
					SWP_NOZORDER);
				EndDeferWindowPos(hdwp);
			}
			break;
		case WM_ERASEBKGND:
			if(g_handler.get() && g_handler->GetBrowserHwnd()){
				// Dont erase the background if the browser window has been loaded
				// (this avoids flashing)
				return 0;
			}
			break;
		case WM_CLOSE:
		  // All clients must forward the WM_CLOSE call to all contained browser
		  // windows to give those windows a chance to cleanup before the window
		  // closes. Don't forward this message if you are cancelling the request.
		  if(g_handler.get())
		  {
			HWND hWnd = g_handler->GetBrowserHwnd();
			if (hWnd)
			  ::SendMessage(hWnd, WM_CLOSE, 0, 0);
		  }
		  break;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
}
