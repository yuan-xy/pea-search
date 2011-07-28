#define UNICODE
#define _UNICODE

#include "../3rd/cef_binary_r78_VS2005/include/cef.h"
#include <sstream>


#define MAX_LOADSTRING 100
#define MAX_URL_LENGTH  255

// Define this value to run CEF with messages processed using the current
// application's message loop.
//#define TEST_SINGLE_THREADED_MESSAGE_LOOP

HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[]=L"Gigaso_Search"; // the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

// Program entry point function.
int APIENTRY wWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

#ifdef TEST_SINGLE_THREADED_MESSAGE_LOOP
  // Initialize the CEF with messages processed using the current application's
  // message loop.
  CefInitialize(false, std::wstring());
#else
  // Initialize the CEF with messages processed using a separate UI thread.
  CefInitialize(true, std::wstring());
#endif
  
  MSG msg;

  MyRegisterClass(hInstance);

  if (!InitInstance (hInstance, nCmdShow))
  {
    return FALSE;
  }

  // Main message loop
  while (GetMessage(&msg, NULL, 0, 0))
  {
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

ATOM MyRegisterClass(HINSTANCE hInstance)
{
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

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle,
      WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, 0, 1000,
      680, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


class ClientHandler : public CefThreadSafeBase<CefHandler>
{
public:
  ClientHandler()
  {
    m_MainHwnd = NULL;
    m_BrowserHwnd = NULL;
    m_EditHwnd = NULL;
    m_bLoading = false;
    m_bCanGoBack = false;
    m_bCanGoForward = false;
  }

  ~ClientHandler()
  {
  }

  // Event called before a new window is created. The |parentBrowser| parameter
  // will point to the parent browser window, if any. The |popup| parameter
  // will be true if the new window is a popup window. If you create the window
  // yourself you should populate the window handle member of |createInfo| and
  // return RV_HANDLED.  Otherwise, return RV_CONTINUE and the framework will
  // create the window.  By default, a newly created window will recieve the
  // same handler as the parent window.  To change the handler for the new
  // window modify the object that |handler| points to.
  virtual RetVal HandleBeforeCreated(CefRefPtr<CefBrowser> parentBrowser,
                                     CefWindowInfo& createInfo, bool popup,
                                     CefRefPtr<CefHandler>& handler,
                                     std::wstring& url)
  {
    return RV_CONTINUE;
  }

  // Event called after a new window is created. The return value is currently
  // ignored.
  virtual RetVal HandleAfterCreated(CefRefPtr<CefBrowser> browser)
  {
    Lock();
    if(!browser->IsPopup())
    {
      // We need to keep the main child window, but not popup windows
      m_Browser = browser;
      m_BrowserHwnd = browser->GetWindowHandle();
    }
    Unlock();
    return RV_CONTINUE;
  }

  // Event called when the address bar changes. The return value is currently
  // ignored.
  virtual RetVal HandleAddressChange(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefFrame> frame,
                                     const std::wstring& url)
  {
    if(m_BrowserHwnd == browser->GetWindowHandle() && frame->IsMain())
    {
      // Set the edit window text
      SetWindowText(m_EditHwnd, url.c_str());
    }
    return RV_CONTINUE;
  }

  // Event called when the page title changes. The return value is currently
  // ignored.
  virtual RetVal HandleTitleChange(CefRefPtr<CefBrowser> browser,
                                   const std::wstring& title)
  {
    // Set the frame window title bar
    HWND hwnd = browser->GetWindowHandle();
    if(!browser->IsPopup())
    {
      // The frame window will be the parent of the browser window
      hwnd = GetParent(hwnd);
    }
    SetWindowText(hwnd, title.c_str());
    return RV_CONTINUE;
  }

  // Event called before browser navigation. The client has an opportunity to
  // modify the |request| object if desired.  Return RV_HANDLED to cancel
  // navigation.
  virtual RetVal HandleBeforeBrowse(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefRequest> request,
                                    NavType navType, bool isRedirect)
  {
    return RV_CONTINUE;
  }

  // Event called when the browser begins loading a page.  The |frame| pointer
  // will be empty if the event represents the overall load status and not the
  // load status for a particular frame.  The return value is currently ignored.
  virtual RetVal HandleLoadStart(CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame)
  {
    if(!browser->IsPopup() && !frame.get())
    {
      Lock();
      // We've just started loading a page
      m_bLoading = true;
      m_bCanGoBack = false;
      m_bCanGoForward = false;
      Unlock();
    }
    return RV_CONTINUE;
  }

  // Event called when the browser is done loading a page. The |frame| pointer
  // will be empty if the event represents the overall load status and not the
  // load status for a particular frame. This event will be generated
  // irrespective of whether the request completes successfully. The return
  // value is currently ignored.
  virtual RetVal HandleLoadEnd(CefRefPtr<CefBrowser> browser,
                               CefRefPtr<CefFrame> frame)
  {
    if(!browser->IsPopup() && !frame.get())
    {
      Lock();
      // We've just finished loading a page
      m_bLoading = false;
      m_bCanGoBack = browser->CanGoBack();
      m_bCanGoForward = browser->CanGoForward();
      Unlock();
    }
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
  virtual RetVal HandleBeforeWindowClose(CefRefPtr<CefBrowser> browser)
  {
    if(m_BrowserHwnd == browser->GetWindowHandle())
    {
      // Free the browser pointer so that the browser can be destroyed
      m_Browser = NULL;
    }
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
	  if(code == VK_ESCAPE && modifiers==0){
		  PostQuitMessage(0);
		  ExitProcess(0);
		  return RV_HANDLED;
	  }
    return RV_CONTINUE;
  }

  // Retrieve the current navigation state flags
  void GetNavState(bool &isLoading, bool &canGoBack, bool &canGoForward)
  {
    Lock();
    isLoading = m_bLoading;
    canGoBack = m_bCanGoBack;
    canGoForward = m_bCanGoForward;
    Unlock();
  }

  void SetMainHwnd(HWND hwnd)
  {
    Lock();
    m_MainHwnd = hwnd;
    Unlock();
  }
  HWND GetMainHwnd() { return m_MainHwnd; }

  void SetEditHwnd(HWND hwnd)
  {
    Lock();
    m_EditHwnd = hwnd;
    Unlock();
  }

  CefRefPtr<CefBrowser> GetBrowser()
  {
    return m_Browser;
  }

  HWND GetBrowserHwnd()
  {
    return m_BrowserHwnd;
  }

protected:
  // The child browser window
  CefRefPtr<CefBrowser> m_Browser;

  // The main frame window handle
  HWND m_MainHwnd;

  // The child browser window handle
  HWND m_BrowserHwnd;

  // The edit window handle
  HWND m_EditHwnd;

  // True if the page is currently loading
  bool m_bLoading;
  // True if the user can navigate backwards
  bool m_bCanGoBack;
  // True if the user can navigate forwards
  bool m_bCanGoForward;
};

// global handler instance
CefRefPtr<ClientHandler> g_handler;

CefRefPtr<CefBrowser> AppGetBrowser()
{
  if(!g_handler.get())
    return NULL;
  return g_handler->GetBrowser();
}

HWND AppGetMainHwnd()
{
  if(!g_handler.get())
    return NULL;
  return g_handler->GetMainHwnd();
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
  {
	switch (message)
	  {
    case WM_CREATE:
      {
        // Create the single static handler class instance
        g_handler = new ClientHandler();
        g_handler->SetMainHwnd(hWnd);
        RECT rect;
        GetClientRect(hWnd, &rect);
        CefWindowInfo info;
        info.SetAsChild(hWnd, rect);
        CefBrowser::CreateBrowser(info, false,
            static_cast<CefRefPtr<CefHandler>>(g_handler),
			L"file://c:/gigaso/web/search.htm");
      }
      return 0;
	case WM_KEYUP:
		switch (wParam) {
			case VK_F5:
				//exec_js(L"refresh");
				break;
			case VK_ESCAPE:
				PostQuitMessage(0);
				ExitProcess(0);
				return 0;
		}
		break;
	  case WM_PAINT:
		  hdc = BeginPaint(hWnd, &ps);
		  EndPaint(hWnd, &ps);
		  return 0;

    case WM_SETFOCUS:
      if(g_handler.get() && g_handler->GetBrowserHwnd())
      {
        // Pass focus to the browser window
        PostMessage(g_handler->GetBrowserHwnd(), WM_SETFOCUS, wParam, NULL);
      }
      return 0;

    case WM_SIZE:
      if(g_handler.get() && g_handler->GetBrowserHwnd())
      {
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
      if(g_handler.get() && g_handler->GetBrowserHwnd())
      {
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
}
