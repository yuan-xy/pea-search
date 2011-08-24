// Copyright (c) 2008-2009 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef_js.h"
#include "client_handler.h"

extern CefRefPtr<ClientHandler> g_handler;

// Implementation of the V8 handler class for the "cef.gigaso" extension.
class ClientV8ExtensionHandler : public CefV8Handler
{
public:
  ClientV8ExtensionHandler() : test_param_("An initial string value.") {}
  virtual ~ClientV8ExtensionHandler() {}

  // Execute with the specified argument list and return value.  Return true if
  // the method was handled.
  virtual bool Execute(const CefString& name,
                       CefRefPtr<CefV8Value> object,
                       const CefV8ValueList& arguments,
                       CefRefPtr<CefV8Value>& retval,
                       CefString& exception)
  {
    if(name == "ZoomIn")
    {
        CefRefPtr<CefBrowser> browser;
		if(g_handler.get()){
          browser = g_handler->GetBrowser();
          if(browser.get())
            browser->SetZoomLevel(browser->GetZoomLevel() + 0.5);
		}
      return true;
    }
    else if(name == "ZoomOut")
    {
        CefRefPtr<CefBrowser> browser;
		if(g_handler.get()){
          browser = g_handler->GetBrowser();
          if(browser.get())
            browser->SetZoomLevel(browser->GetZoomLevel() - 0.5);
		}
      return true;
    }
    else if(name == "ZoomReset")
    {
        CefRefPtr<CefBrowser> browser;
		if(g_handler.get()){
          browser = g_handler->GetBrowser();
          if(browser.get())
            browser->SetZoomLevel(0.0);
		}
      return true;
    }
    else if(name == "ShowDevTools")
    {
        CefRefPtr<CefBrowser> browser;
		if(g_handler.get()){
          browser = g_handler->GetBrowser();
          if(browser.get())
            browser->ShowDevTools();
		}
      return true;
    }
    else if(name == "RegPlugin")
    {
		UINT u = WinExec("regsvr32 /s npPluginGigaso.dll",SW_HIDE);
		if(u>31){
			Sleep(10);
			WinExec("search.exe",SW_SHOW);
			PostQuitMessage(0);
			TerminateProcess(GetCurrentProcess(),0);
		}
		return true;
	  return true;
    }
    return false;
  }

private:
  CefString test_param_;

  IMPLEMENT_REFCOUNTING(ClientV8ExtensionHandler);
};


void InitExtensionTest()
{
  // Register a V8 extension with the below JavaScript code that calls native
  // methods implemented in ClientV8ExtensionHandler.
  std::string code = "var cef;"
    "if (!cef)"
    "  cef = {};"
    "if (!cef.gigaso)"
    "  cef.gigaso = {};"
    "(function() {"
    "  cef.gigaso.zoom_in = function() {"
    "    native function ZoomIn();"
    "    return ZoomIn();"
    "  };"
    "  cef.gigaso.zoom_out = function() {"
    "    native function ZoomOut();"
    "    return ZoomOut();"
    "  };"
    "  cef.gigaso.zoom_reset = function() {"
    "    native function ZoomReset();"
    "    return ZoomReset();"
    "  };"
    "  cef.gigaso.dev_tool = function() {"
    "    native function ShowDevTools();"
    "    return ShowDevTools();"
    "  };"
    "  cef.gigaso.reg_plugin = function() {"
    "    native function RegPlugin();"
    "    return RegPlugin();"
    "  };"
	"})();";
  CefRegisterExtension("v8/gigaso", code, new ClientV8ExtensionHandler());
}
