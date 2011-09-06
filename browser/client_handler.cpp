#include "../3rd/cef_binary/include/cef.h"
#include "../filesearch/common.h"
#include "client_handler.h"
#include "string_util.h"
#include <sstream>
#include <stdio.h>
#include <string>
#include "html_source.h"

extern CefRefPtr<ClientHandler> g_handler;


void exec_js_str(wchar_t *str){
	if(g_handler.get()){
		CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();
		if(browser.get()){
			browser->GetMainFrame()->ExecuteJavaScript(
				str, L"about:blank", 0);
		}
	}
}

void exec_js(const wchar_t *function_name){
	wchar_t buffer[100];
	wsprintf(buffer, L"try{if(%s) %s()}catch(e){}",function_name,function_name);
	exec_js_str(buffer);
}

static void init_dir(){
	wchar_t buffer[MAX_PATH*2];
	wchar_t *cmdLine = GetCommandLineW();
	std::wstring str(cmdLine), olds(L"\\"), news(L"\\\\");
	str = StringReplace(str,olds,news);
	wsprintf(buffer,L"try{if(init_dir) init_dir('%s');}catch(e){}",str.c_str());
	exec_js_str(buffer);
}

static void load_str(const CefString& url){
	if(g_handler.get()){
		CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();
		if(browser.get()){
			browser->GetMainFrame()->LoadStringW(web_source,url);
		}
	}
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
	if(wcscmp(frame->GetURL().ToWString().c_str(),L"about:blank")==0){
		wchar_t full_path[MAX_PATH];
		get_abs_path(L"web\\search2.htm",full_path);
		load_str(full_path);
	}else{
		init_dir();
	}
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

/*
DWORD WINAPI upgrade_request(void *p){
  CefRefPtr<CefBrowser> browser;
  if(g_handler.get()){
    browser = g_handler->GetBrowser();
	if(browser.get()){
		  CefRefPtr<CefRequest> request(CefRequest::CreateRequest());
		  request->SetURL("http://www.1dooo.com:3000/upgrades");
		  CefRefPtr<CefPostDataElement> postDataElement(
			  CefPostDataElement::CreatePostDataElement());
		  std::string data = "upgrade[os]=os&upgrade[cpu]=cpu&upgrade[disk]=disk&upgrade[ver]=ver&upgrade[user]=user";
		  postDataElement->SetToBytes(data.length(), data.c_str());
		  CefRefPtr<CefPostData> postData(CefPostData::CreatePostData());
		  postData->AddElement(postDataElement);
		  request->SetPostData(postData);
		  browser->GetMainFrame()->LoadRequest(request);
		  return 0;
	}
  }
  return 1;
}


void upgrade_thread(){
	CreateThread(NULL,0,upgrade_request,NULL,0,NULL);
}
*/