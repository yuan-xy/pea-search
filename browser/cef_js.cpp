#include "env.h"
#include "cef_js.h"
#include "client_handler.h"
#include "common.h"
#include <Shlobj.h>

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
    if(name == "SelectDir")
    {
		BROWSEINFO bi;
		WCHAR Buffer[MAX_PATH];
		bi.hwndOwner = NULL;
		bi.pidlRoot = NULL;
		bi.pszDisplayName = Buffer;
		bi.lpszTitle = L"选择目录";
		bi.ulFlags = BIF_RETURNONLYFSDIRS;
		bi.lpfn = NULL;
		bi.iImage = 0;
		LPITEMIDLIST pIDList = SHBrowseForFolder(&bi);
		if(pIDList){
			SHGetPathFromIDList(pIDList, Buffer);
			retval = CefV8Value::CreateString(Buffer);
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
    else if(name == "DoUpdate")
    {
      if(arguments.size() != 1 || !arguments[0]->IsString())
        return false;
		int h = (int)ShellExecuteA(NULL,"open",arguments[0]->GetStringValue().ToString().c_str(),NULL,NULL,SW_SHOWNORMAL);
		if(h>32){
			retval = CefV8Value::CreateBool(true);
		}else{
			retval = CefV8Value::CreateBool(false);
		}
		return true;
    }
    else if(name == "CrashTest")
    {
		int *i = (int*) 0x45;  
        *i = 5;  // crash!  
	    return true;
    }
    else if(name == "GetOs"){
	  WCHAR buffer[128];
	  get_os(buffer);
      retval = CefV8Value::CreateString(buffer);
      return true;
    }
    else if(name == "GetCpu"){
	  WCHAR buffer[128];
	  get_cpu(buffer);
      retval = CefV8Value::CreateString(buffer);
      return true;
    }
    else if(name == "GetDisk"){
	  WCHAR buffer[128];
	  get_disk(buffer);
      retval = CefV8Value::CreateString(buffer);
      return true;
    }
    else if(name == "GetVer"){
	  WCHAR buffer[128];
	  get_ver(buffer);
      retval = CefV8Value::CreateString(buffer);
      return true;
    }
    else if(name == "GetUser"){
	  WCHAR buffer[128];
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
    "  cef.gigaso.dev_tool = function() {"
    "    native function ShowDevTools();"
    "    return ShowDevTools();"
    "  };"
    "  cef.gigaso.selectDir = function() {"
    "    native function SelectDir();"
    "    return SelectDir();"
    "  };"
    "  cef.gigaso.do_update = function(b) {"
    "    native function DoUpdate();"
    "    return DoUpdate(b);"
    "  };"
    "  cef.gigaso.crash_test = function() {"
    "    native function CrashTest();"
    "    return CrashTest();"
    "  };"
	"})();";
  CefRegisterExtension("v8/gigaso", code, new ClientV8ExtensionHandler());
}
