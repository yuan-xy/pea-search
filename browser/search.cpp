#define UNICODE
#define _UNICODE

#include <string.h>
#include <stdio.h>
#include <windows.h>
#include "webform.h"

HINSTANCE hInstance;
HWND hMain; // Our main window
HWND hwebf; // We declare this handle globally, just for convenience
bool loaded, isquit = false; // we declare it as a global variable, for convenience

void PumpMessages() {
	MSG msg;
	while (true) {
		BOOL res = PeekMessage(&msg, 0, 0, 0, PM_REMOVE);
		if (!res)
			return;
		if (msg.message == WM_QUIT) {
			isquit = true;
			return;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

static void show_title(){
		IHTMLDocument2 *doc = WebformGetDoc(hwebf);
		BSTR b = 0;
		doc->get_title(&b);
		MessageBoxW(hMain, b, L"Title:", MB_OK);
		if (b != 0)
			SysFreeString(b);
		doc->Release();
}

static void exec_js(){
		IHTMLDocument2 *doc = WebformGetDoc(hwebf);
		IHTMLWindow2 *win = 0;
		doc->get_parentWindow(&win);
		if (win != 0) {
			BSTR cmd = SysAllocString(
					L"if(search_if_change) search_if_change()");
			VARIANT v;
			VariantInit(&v);
			win->execScript(cmd, NULL, &v);
			VariantClear(&v);
			SysFreeString(cmd);
			win->Release();
		}
		doc->Release();
}

static void dom_demo(){
		// And alter the text: document.all["id"].innerHTML="fred<br>,ary";
		IHTMLElement *e2 = WebformGetElement < IHTMLElement > (hwebf, _T("id"));
		if (e2 != 0) {
			e2->put_innerHTML(L"hello,<br>Ô¬ÐÂÓî<hr>world");
			e2->Release();
		}
}

static void get_path(){
	WCHAR szPath[MAX_PATH]; 
	if( !GetModuleFileName( NULL, szPath, MAX_PATH ) ){
			//MessageBox(_T("GetModuleFileName failed!") + GetLastError()); 
			return;
	}
	WCHAR *p = wcsrchr(szPath,'\\');
	*(p+1) = L'\0';
	wcscat(p,L"web\\search.htm");
	//MessageBox(NULL, szPath, szPath, MB_OK);
	SetWindowText(hwebf, szPath);
}

LRESULT CALLBACK PlainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CREATE: {
		hwebf = CreateWindow(WEBFORM_CLASS, _T(""),
				WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | WS_VSCROLL, 0, 0,
				100, 100, hwnd, (HMENU) 103, hInstance, 0);
		get_path();
		//SetTimer(hwnd,1,2000,0);
	}
		break;
	case WM_TIMER: {
		KillTimer(hwnd, 1);
		loaded = false;
		WebformReady(hwebf);
		WebformSet(hwebf,_T("<html><head><script type='text/javascript'>\n")
				_T("function MyJavascriptFunc(arg) {alert(arg);}\n")
				_T("</script></head><body><form>\n")
				_T("<input name='cx' type='checkbox' checked> Do you want some?<br>\n")
				_T("<input name='tx' type='text' value='Come and get it!'><br>\n")
				_T("<input name='sub' type='submit' value='Ok'>\n")
				_T("<input name='sub' type='submit' value='Cancel'>\n")
				_T("<br><div id='id'>hell<div>\n")
				_T("</form></body></html>\n"));
		WebformGo(hwebf, 0);
         while (!loaded && !isquit) {
                 PumpMessages();
                 Sleep(5);
         }
         if (isquit) {
                 PostQuitMessage(0);
                 return 0;
         }
		 dom_demo();
	}
		break;
	case WM_SIZE: {
		MoveWindow(hwebf, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
	}
		break;
	case WM_PAINT: {
		PAINTSTRUCT ps;
		BeginPaint(hwnd, &ps);
		FillRect(ps.hdc, &ps.rcPaint, (HBRUSH) GetStockObject(WHITE_BRUSH));
		EndPaint(hwnd, &ps);
		return 0;
	}
	case WM_COMMAND: {
		int id = LOWORD(wParam), code = HIWORD(wParam);
		if (id == 103 && code == WEBFN_LOADED) {
			loaded = true;
			//show_title();
			//exec_js();
		} else if (id == 103 && code == WEBFN_CLICKED) {
			const TCHAR *url = WebformLastClick(hwebf);
			WebformGo(hwebf, url);
		}
	}
		break;
	case WM_KEYUP: 
		exec_js();
		break;
	case WM_DESTROY: {
		PostQuitMessage(0);
	}
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE h,HINSTANCE,LPSTR,int)
{	hInstance=h;
	OleInitialize(0);
	//
	WNDCLASSEX wcex; ZeroMemory(&wcex,sizeof(wcex)); wcex.cbSize = sizeof(WNDCLASSEX);
	BOOL res=GetClassInfoEx(hInstance,_T("PlainClass"),&wcex);
	if (!res)
	{	wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = (WNDPROC)PlainWndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = NULL;
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = NULL;
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = _T("PlainClass");
		wcex.hIconSm = NULL;
		ATOM res=RegisterClassEx(&wcex);
		if (res==0) {MessageBox(NULL,_T("Failed to register class"),_T("Error"),MB_OK); return 0;}
	}
	//
	hMain = CreateWindowEx(0,_T("PlainClass"), _T("Plain Window"), WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,
			CW_USEDEFAULT, CW_USEDEFAULT, 400, 400, NULL, NULL, hInstance, NULL);
	if (hMain==NULL) {MessageBox(NULL,_T("Failed to create window"),_T("Error"),MB_OK); return 0;}
	ShowWindow(hMain,SW_SHOW);
	//
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{	TranslateMessage(&msg);
		// The problem is that keypresses won't always go to the main window.
		// So we insert this into our global message loop, to route them there.
		if (msg.hwnd!=hMain && msg.message>=WM_KEYFIRST && msg.message<=WM_KEYLAST) SendMessage(hMain, msg.message, msg.wParam, msg.lParam);
		DispatchMessage(&msg);
	}
	//
	OleUninitialize();
	return (int)msg.wParam;
}

