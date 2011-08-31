// Copyright (c) 2008-2009 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "env.h"
#include "cef_js.h"
#include "client_handler.h"
#include "common.h"

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
    }
    else if(name == "CheckUpdate")
    {
		int status=UPDATE_CHECH_UNKNOWN;
		FILE *file;
		if ((file = fopen(UPDATE_CHECH_FILE, "r+")) == NULL) return true;
		if(fread(&status,sizeof(int),1,file)==1){
			if(status==UPDATE_CHECH_NEW){
				char fname[MAX_PATH] = {0};
				size_t ret = fread(fname,sizeof(char),MAX_PATH,file);
				if(ret==MAX_PATH){
					int h = (int)ShellExecuteA(NULL,"open",fname,NULL,NULL,SW_SHOWNORMAL);
					if(h>32){
						status = UPDATE_CHECH_DONE;
						fseek(file,0,SEEK_SET);
						fwrite(&status,sizeof(int),1,file);
					}
				}
			}
		}
		retval = CefV8Value::CreateInt(status);
		fclose (file);
		return true;
    }
    else if(name == "CrashTest")
    {
		int *i = (int*) 0x45;  
        *i = 5;  // crash!  
	    return true;
    }
    else if(name == "GetOs"){
	  wchar_t buffer[128];
	  get_os(buffer);
      retval = CefV8Value::CreateString(buffer);
      return true;
    }
    else if(name == "GetCpu"){
	  wchar_t buffer[128];
	  get_cpu(buffer);
      retval = CefV8Value::CreateString(buffer);
      return true;
    }
    else if(name == "GetDisk"){
	  wchar_t buffer[128];
	  get_disk(buffer);
      retval = CefV8Value::CreateString(buffer);
      return true;
    }
    else if(name == "GetVer"){
	  wchar_t buffer[128];
	  get_ver(buffer);
      retval = CefV8Value::CreateString(buffer);
      return true;
    }
    else if(name == "GetUser"){
	  wchar_t buffer[128];
	  get_user(buffer);
      retval = CefV8Value::CreateString(buffer);
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
    "  cef.gigaso.__defineGetter__('os', function() {"
    "    native function GetOs();"
    "    return GetOs();"
    "  });"
    "  cef.gigaso.__defineGetter__('cpu', function() {"
    "    native function GetCpu();"
    "    return GetCpu();"
    "  });"
    "  cef.gigaso.__defineGetter__('disk', function() {"
    "    native function GetDisk();"
    "    return GetDisk();"
    "  });"
    "  cef.gigaso.__defineGetter__('ver', function() {"
    "    native function GetVer();"
    "    return GetVer();"
    "  });"
    "  cef.gigaso.__defineGetter__('user', function() {"
    "    native function GetUser();"
    "    return GetUser();"
    "  });"
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
    "  cef.gigaso.check_update = function() {"
    "    native function CheckUpdate();"
    "    return CheckUpdate();"
    "  };"
    "  cef.gigaso.crash_test = function() {"
    "    native function CrashTest();"
    "    return CrashTest();"
    "  };"
	"})();";
  CefRegisterExtension("v8/gigaso", code, new ClientV8ExtensionHandler());
}
