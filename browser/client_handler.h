#ifndef _CLIENT_HANDLER_H
#define _CLIENT_HANDLER_H

#include "../3rd/cef_binary/include/cef.h"
#include "cef_util.h"


// Define this value to redirect all popup URLs to the main application browser
// window.
//#define TEST_REDIRECT_POPUP_URLS


// ClientHandler implementation.
class ClientHandler : public CefClient,
                      public CefLifeSpanHandler,
                      public CefLoadHandler,
					  public CefKeyboardHandler
{
public:
  ClientHandler();
  virtual ~ClientHandler();

  virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE
      { return this; }
  virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE
      { return this; }
  
    // CefLifeSpanHandler methods
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;

  virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefFrame> frame,
                         int httpStatusCode) OVERRIDE;
  virtual bool OnLoadError(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           ErrorCode errorCode,
                           const CefString& failedUrl,
                           CefString& errorText) OVERRIDE;

  virtual bool OnKeyEvent(CefRefPtr<CefBrowser> browser,
                          KeyEventType type,
                          int code,
                          int modifiers,
                          bool isSystemKey) OVERRIDE;


  CefRefPtr<CefBrowser> GetBrowser() { return m_Browser; }
  CefWindowHandle GetBrowserHwnd() { return m_BrowserHwnd; }


protected:
  CefRefPtr<CefBrowser> m_Browser;
  CefWindowHandle m_BrowserHwnd;

  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(ClientHandler);
  // Include the default locking implementation.
  IMPLEMENT_LOCKING(ClientHandler);
};

#endif // _CLIENT_HANDLER_H
