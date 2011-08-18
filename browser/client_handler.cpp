#include "../3rd/cef_binary/include/cef.h"
#include "client_handler.h"
#include "string_util.h"
#include <sstream>
#include <stdio.h>
#include <string>

extern CefRefPtr<ClientHandler> g_handler;


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

static void init_dir(){
	wchar_t buffer[MAX_PATH*2];
	wchar_t *cmdLine = GetCommandLineW();
	std::wstring str(cmdLine), olds(L"\\"), news(L"\\\\");
	str = StringReplace(str,olds,news);
	wsprintf(buffer,L"try{in_exe=true;if(init_dir) init_dir('%s');}catch(e){}",str.c_str());
	exec_js_str(buffer);
}

ClientHandler::ClientHandler(){}
ClientHandler::~ClientHandler(){}

void ClientHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
  REQUIRE_UI_THREAD();

  AutoLock lock_scope(this);
  if(!browser->IsPopup())
  {
    // We need to keep the main child window, but not popup windows
    m_Browser = browser;
    m_BrowserHwnd = browser->GetWindowHandle();
  }
}



void ClientHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              int httpStatusCode)
{
	init_dir();
}

bool ClientHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                ErrorCode errorCode,
                                const CefString& failedUrl,
                                CefString& errorText)
{
  REQUIRE_UI_THREAD();
    std::stringstream ss;
    ss <<       "<html><head><title>Load Failed</title></head>"
                "<body><h1>Load Failed</h1>"
                "<h2>Load of URL " << std::string(failedUrl) <<
                " failed with error code " << static_cast<int>(errorCode) <<
                ".</h2></body>"
                "</html>";
    errorText = ss.str();
  return false;
}


bool ClientHandler::OnKeyEvent(CefRefPtr<CefBrowser> browser,
                          KeyEventType type,
                          int code,
                          int modifiers,
                          bool isSystemKey)

	{
		if(code == VK_ESCAPE && type==KEYEVENT_RAWKEYDOWN){
			PostQuitMessage(0);
			TerminateProcess(GetCurrentProcess(),0);
			return true;
		}
		if(code == VK_F5 && type==KEYEVENT_RAWKEYDOWN){
			exec_js(L"refresh");
			return true;
		}
		return false;
	}