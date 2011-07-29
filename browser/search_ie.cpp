#define UNICODE
#define _UNICODE

#include <string.h>
#include <stdio.h>
#include <windows.h>
#include <Shlwapi.h>
#include "webform.h"
#include "../filesearch/common.h"

#include <string>

void searchAndReplace(std::wstring& value, std::wstring const& search,std::wstring const& replace)
{
    std::wstring::size_type  next;

    for(next = value.find(search);        // Try and find the first match
        next != std::wstring::npos;        // next is npos if nothing was found
        next = value.find(search,next)    // search for the next match starting after
                                          // the last match that was found.
       )
    {
        // Inside the loop. So we found a match.
        value.replace(next,search.length(),replace);   // Do the replacement.
        next += replace.length();                      // Move to just after the replace
                                                       // This is the point were we start
                                                       // the next search from. 
    }
}

HINSTANCE hInstance;
HWND hMain; // Our main window
HWND hwebf; // We declare this handle globally, just for convenience
bool loaded, isquit = false; // we declare it as a global variable, for convenience

void clear(){
	if(hwebf != NULL) WebformDestroy(hwebf);
	hwebf = NULL;
	loaded = false;
}

static void load(){
	WCHAR szPath[MAX_PATH];
	get_abs_path(L"web\\search.htm", szPath);
	SetWindowText(hwebf, szPath);
}

void init_browser(HWND hwnd){
	hwebf = WebformCreate(hwnd,103);
	load();
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

static void exec_js_str(const wchar_t *str){
		IHTMLDocument2 *doc = WebformGetDoc(hwebf);
		IHTMLWindow2 *win = 0;
		doc->get_parentWindow(&win);
		if (win != 0) {
			BSTR cmd = SysAllocString(str);
			VARIANT v;
			VariantInit(&v);
			win->execScript(cmd, NULL, &v);
			VariantClear(&v);
			SysFreeString(cmd);
			win->Release();
		}
		doc->Release();
}

static void exec_js(const wchar_t *function_name){
		wchar_t buffer[100];
		wsprintf(buffer, L"try{if(%s) %s()}catch(e){}",function_name,function_name);
		exec_js_str(buffer);
}

static void init_dir(){
	wchar_t buffer[MAX_PATH*2];
	wchar_t *cmdLine = GetCommandLineW();
	std::wstring str(cmdLine), olds(L"\\"), news(L"\\\\");
	searchAndReplace(str,olds,news);
	wsprintf(buffer,L"try{init_dir('%s');in_exe=true;}catch(e){}",str.c_str());
	exec_js_str(buffer);
}


LRESULT CALLBACK PlainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CREATE: {
		init_browser(hwnd);
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
			init_dir();
		} else if (id == 103 && code == WEBFN_CLICKED) {
			const TCHAR *url = WebformLastClick(hwebf);
			WebformGo(hwebf, url);
		}
	}
		break;
	case WM_KEYUP:
		switch (wParam) {
			case VK_F5:
				exec_js(L"refresh");
				break;
			case VK_ESCAPE:
				PostQuitMessage(0);
				return 0;
		}
		break;
	case WM_DESTROY: {
		PostQuitMessage(0);
	}
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}


void TranslateAccKey(LPMSG lpMsg) {
	if(lpMsg->wParam==VK_F5 || lpMsg->wParam==VK_ESCAPE){
		if(lpMsg->message>=WM_KEYFIRST && lpMsg->message<=WM_KEYLAST){
			SendMessage(hMain, lpMsg->message, lpMsg->wParam, lpMsg->lParam);
		}
		return;
	}
	IWebBrowser2 *ibrowser = WebformGetBrowser(hwebf);
	if (ibrowser != NULL) {
		IOleInPlaceActiveObject *cpc = NULL;
		ibrowser->QueryInterface(IID_IOleInPlaceActiveObject, (void**) &cpc);
		if (cpc != 0) {
			cpc->TranslateAccelerator(lpMsg);
			cpc->Release();
		}
		ibrowser->Release();
		ibrowser = NULL;
	}
}

int WINAPI WinMain(HINSTANCE h,HINSTANCE hPrevInstance,LPSTR s,int nCmdShow){
	setPWD(NULL);
	hInstance=h;
	OleInitialize(0);
	WNDCLASSEX wcex; ZeroMemory(&wcex,sizeof(wcex)); wcex.cbSize = sizeof(WNDCLASSEX);
	BOOL res=GetClassInfoEx(hInstance,_T("Gigaso_Search"),&wcex);
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
		wcex.lpszClassName = _T("Gigaso_Search");
		wcex.hIconSm = NULL;
		ATOM res=RegisterClassEx(&wcex);
		if (res==0) {MessageBox(NULL,_T("Failed to register class"),_T("Error"),MB_OK); return 0;}
	}
	//
	hMain = CreateWindowEx(0,_T("Gigaso_Search"), _T("¼Ç¼ÑÎÄ¼þËÑË÷"), WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,
			CW_USEDEFAULT, CW_USEDEFAULT, 1000, 680, NULL, NULL, hInstance, NULL);
	if (hMain==NULL) {MessageBox(NULL,_T("Failed to create window"),_T("Error"),MB_OK); return 0;}
	ShowWindow(hMain,SW_SHOW);
	SetForegroundWindow(hMain);
	SetFocus(hMain);
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)){
		TranslateAccKey(&msg);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	OleUninitialize();
	return (int)msg.wParam;
}

