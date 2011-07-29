#define UNICODE
#define _UNICODE

#include "../3rd/cef_binary_r78_VS2005/include/cef.h"
#include "../filesearch/common.h"
#include "scheme_test.h"
#include <sstream>

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

#define MAX_LOADSTRING 100
#define MAX_URL_LENGTH  255
//#define TEST_SINGLE_THREADED_MESSAGE_LOOP

HINSTANCE hInst;								// current instance
HWND hMainWin;
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[]=L"Gigaso_Search"; // the main window class name

ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

static void exec_js_str(wchar_t *str);
static void exec_js(const wchar_t *function_name);
static void trans_focus();

static void init_dir(){
	wchar_t buffer[MAX_PATH*2];
	wchar_t *cmdLine = GetCommandLineW();
	std::wstring str(cmdLine), olds(L"\\"), news(L"\\\\");
	searchAndReplace(str,olds,news);
	wsprintf(buffer,L"try{init_dir('%s');in_exe=true;}catch(e){alert(e)}",str.c_str());
	exec_js_str(buffer);
}

int APIENTRY wWinMain(HINSTANCE hInstance,
					  HINSTANCE hPrevInstance,
					  LPTSTR    lpCmdLine,
					  int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	setPWD(NULL);

#ifdef TEST_SINGLE_THREADED_MESSAGE_LOOP
	// Initialize the CEF with messages processed using the current application's
	// message loop.
	CefInitialize(false, std::wstring());
#else
	// Initialize the CEF with messages processed using a separate UI thread.
	CefInitialize(true, std::wstring());
#endif
	InitSchemeTest();
	MyRegisterClass(hInstance);
	if (!InitInstance (hInstance, nCmdShow)) return FALSE;
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
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= NULL;
	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow){
	hInst = hInstance;
	hMainWin = CreateWindow(szWindowClass, szTitle,
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, 0, 1000,
		680, NULL, NULL, hInstance, NULL);
	if (!hMainWin) return FALSE;
	ShowWindow(hMainWin, nCmdShow);
	UpdateWindow(hMainWin);
	return TRUE;
}


class ClientHandler : public CefThreadSafeBase<CefHandler>
{
public:
	ClientHandler(){
		m_bLoading = false;
	}

	~ClientHandler(){}

	virtual RetVal HandleBeforeCreated(CefRefPtr<CefBrowser> parentBrowser,
		CefWindowInfo& createInfo, bool popup,
		CefRefPtr<CefHandler>& handler,
		std::wstring& url){
		return RV_CONTINUE;
	}

	virtual RetVal HandleAfterCreated(CefRefPtr<CefBrowser> browser){
		m_Browser = browser;
		m_BrowserHwnd = browser->GetWindowHandle(); 
		return RV_CONTINUE;
	}

	virtual RetVal HandleAddressChange(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		const std::wstring& url){
		return RV_CONTINUE;
	}

	virtual RetVal HandleTitleChange(CefRefPtr<CefBrowser> browser,
		const std::wstring& title)
	{
		return RV_CONTINUE;
	}

	virtual RetVal HandleBeforeBrowse(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefRequest> request,
		NavType navType, bool isRedirect)
	{
		return RV_CONTINUE;
	}

	virtual RetVal HandleLoadStart(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame)
	{
		m_bLoading = true;
		return RV_CONTINUE;
	}

	virtual RetVal HandleLoadEnd(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame)
	{
		m_bLoading = false;
		init_dir();
		return RV_CONTINUE;
	}

	// Called when the browser fails to load a resource.  |errorCode| is the
	// error code number and |failedUrl| is the URL that failed to load.  To
	// provide custom error text assign the text to |errorText| and return
	// RV_HANDLED.  Otherwise, return RV_CONTINUE for the default error text.
	virtual RetVal HandleLoadError(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		ErrorCode errorCode,
		const std::wstring& failedUrl,
		std::wstring& errorText)
	{
		if(errorCode == ERR_CACHE_MISS)
		{
			// Usually caused by navigating to a page with POST data via back or
			// forward buttons.
			errorText = L"<html><head><title>Expired Form Data</title></head>"
				L"<body><h1>Expired Form Data</h1>"
				L"<h2>Your form request has expired. "
				L"Click reload to re-submit the form data.</h2></body>"
				L"</html>";
		}
		else
		{
			// All other messages.
			std::wstringstream ss;
			ss <<       L"<html><head><title>Load Failed</title></head>"
				L"<body><h1>Load Failed</h1>"
				L"<h2>Load of URL " << failedUrl <<
				L"failed with error code " << static_cast<int>(errorCode) <<
				L".</h2></body>"
				L"</html>";
			errorText = ss.str();
		}
		return RV_HANDLED;
	}

	// Event called before a resource is loaded.  To allow the resource to load
	// normally return RV_CONTINUE. To redirect the resource to a new url
	// populate the |redirectUrl| value and return RV_CONTINUE.  To specify
	// data for the resource return a CefStream object in |resourceStream|, set
	// 'mimeType| to the resource stream's mime type, and return RV_CONTINUE.
	// To cancel loading of the resource return RV_HANDLED.
	virtual RetVal HandleBeforeResourceLoad(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefRequest> request,
		std::wstring& redirectUrl,
		CefRefPtr<CefStreamReader>& resourceStream,
		std::wstring& mimeType,
		int loadFlags)
	{
		return RV_CONTINUE;
	}

	// Event called before a context menu is displayed.  To cancel display of the
	// default context menu return RV_HANDLED.
	virtual RetVal HandleBeforeMenu(CefRefPtr<CefBrowser> browser,
		const MenuInfo& menuInfo)
	{
		return RV_CONTINUE;
	}


	// Event called to optionally override the default text for a context menu
	// item.  |label| contains the default text and may be modified to substitute
	// alternate text.  The return value is currently ignored.
	virtual RetVal HandleGetMenuLabel(CefRefPtr<CefBrowser> browser,
		MenuId menuId, std::wstring& label)
	{
		return RV_CONTINUE;
	}

	// Event called when an option is selected from the default context menu.
	// Return RV_HANDLED to cancel default handling of the action.
	virtual RetVal HandleMenuAction(CefRefPtr<CefBrowser> browser,
		MenuId menuId)
	{
		return RV_CONTINUE;
	}

	// Event called to format print headers and footers.  |printInfo| contains
	// platform-specific information about the printer context.  |url| is the
	// URL if the currently printing page, |title| is the title of the currently
	// printing page, |currentPage| is the current page number and |maxPages| is
	// the total number of pages.  Six default header locations are provided
	// by the implementation: top left, top center, top right, bottom left,
	// bottom center and bottom right.  To use one of these default locations
	// just assign a string to the appropriate variable.  To draw the header
	// and footer yourself return RV_HANDLED.  Otherwise, populate the approprate
	// variables and return RV_CONTINUE.
	virtual RetVal HandlePrintHeaderFooter(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefPrintInfo& printInfo,
		const std::wstring& url,
		const std::wstring& title,
		int currentPage, int maxPages,
		std::wstring& topLeft,
		std::wstring& topCenter,
		std::wstring& topRight,
		std::wstring& bottomLeft,
		std::wstring& bottomCenter,
		std::wstring& bottomRight)
	{
		// Place the page title at top left
		topLeft = title;
		// Place the page URL at top right
		topRight = url;

		// Place "Page X of Y" at bottom center
		std::wstringstream strstream;
		strstream << L"Page " << currentPage << L" of " << maxPages;
		bottomCenter = strstream.str();

		return RV_CONTINUE;
	}

	// Run a JS alert message.  Return RV_CONTINUE to display the default alert
	// or RV_HANDLED if you displayed a custom alert.
	virtual RetVal HandleJSAlert(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		const std::wstring& message)
	{
		return RV_CONTINUE;
	}

	// Run a JS confirm request.  Return RV_CONTINUE to display the default alert
	// or RV_HANDLED if you displayed a custom alert.  If you handled the alert
	// set |retval| to true if the user accepted the confirmation.
	virtual RetVal HandleJSConfirm(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		const std::wstring& message, bool& retval)
	{
		return RV_CONTINUE;
	}

	// Run a JS prompt request.  Return RV_CONTINUE to display the default prompt
	// or RV_HANDLED if you displayed a custom prompt.  If you handled the prompt
	// set |retval| to true if the user accepted the prompt and request and
	// |result| to the resulting value.
	virtual RetVal HandleJSPrompt(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		const std::wstring& message,
		const std::wstring& defaultValue,
		bool& retval,
		std::wstring& result)
	{
		return RV_CONTINUE;
	}

	// Called just before a window is closed. The return value is currently
	// ignored.
	virtual RetVal HandleBeforeWindowClose(CefRefPtr<CefBrowser> browser){
		m_Browser = NULL;
		return RV_CONTINUE;
	}

	// Called when the browser component is about to loose focus. For instance,
	// if focus was on the last HTML element and the user pressed the TAB key.
	// The return value is currently ignored.
	virtual RetVal HandleTakeFocus(CefRefPtr<CefBrowser> browser, bool reverse)
	{
		return RV_CONTINUE;
	}

	// Called when the browser component is requesting focus. |isWidget| will be
	// true if the focus is requested for a child widget of the browser window.
	// Return RV_CONTINUE to allow the focus to be set or RV_HANDLED to cancel
	// setting the focus.
	virtual RetVal HandleSetFocus(CefRefPtr<CefBrowser> browser,
		bool isWidget)
	{
		return RV_CONTINUE;
	}

	// Called when the browser component receives a keyboard event.
	// |type| is the type of keyboard event (see |KeyEventType|).
	// |code| is the windows scan-code for the event.
	// |modifiers| is a set of bit-flags describing any pressed modifier keys.
	// |isSystemKey| is set if Windows considers this a 'system key' message;
	//   (see http://msdn.microsoft.com/en-us/library/ms646286(VS.85).aspx)
	// Return RV_HANDLED if the keyboard event was handled or RV_CONTINUE
	// to allow the browser component to handle the event.
	RetVal HandleKeyEvent(CefRefPtr<CefBrowser> browser,
		KeyEventType type,
		int code,
		int modifiers,
		bool isSystemKey)
	{
		if(code == VK_ESCAPE && type==KEYEVENT_RAWKEYDOWN){
			PostQuitMessage(0);
			TerminateProcess(GetCurrentProcess(),0);
			return RV_HANDLED;
		}
		if(code == VK_F5 && type==KEYEVENT_RAWKEYDOWN){
			exec_js(L"refresh");
			return RV_HANDLED;
		}
		return RV_CONTINUE;
	}

	CefRefPtr<CefBrowser> GetBrowser(){
		return m_Browser;
	}

	  HWND GetBrowserHwnd(){
		return m_BrowserHwnd;
	  }
protected:
	// The child browser window
	CefRefPtr<CefBrowser> m_Browser;
  // The child browser window handle
  HWND m_BrowserHwnd;
	// True if the page is currently loading
	bool m_bLoading;

};
CefRefPtr<ClientHandler> g_handler;


static void exec_js_str(wchar_t *str){
	if(g_handler.get()){
		CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();
		if(browser.get()){
			browser->GetMainFrame()->ExecuteJavaScript(
				str, L"about:blank", 0);
		}
	}
}

static void exec_js(const wchar_t *function_name){
	wchar_t buffer[100];
	wsprintf(buffer, L"try{if(%s) %s()}catch(e){}",function_name,function_name);
	exec_js_str(buffer);
}

static void trans_focus(){
	if(g_handler.get() && g_handler->GetBrowserHwnd()){
		PostMessage(g_handler->GetBrowserHwnd(), WM_SETFOCUS, NULL, NULL);
		exec_js_str(L"try{ $('#search').blur();$('#search').focus();}catch(e){}");
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
				CefWindowInfo info;
				info.SetAsChild(hWnd, rect);
				wchar_t full_path[MAX_PATH];
				get_abs_path(L"web\\search.htm",full_path);
				CefBrowser::CreateBrowser(info, false,
					static_cast<CefRefPtr<CefHandler>>(g_handler), full_path);
			}
			return 0;
		case WM_ACTIVATE:
			if((short)wParam!=WA_INACTIVE){
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
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
}
