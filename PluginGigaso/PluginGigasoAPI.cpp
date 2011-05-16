/**********************************************************\

  Auto-generated PluginGigasoAPI.cpp

\**********************************************************/

#include "JSObject.h"
#include "variant_list.h"
#include "DOM/Document.h"

#include "PluginGigasoAPI.h"

#include "sharelib.h"
#include <stdlib.h>   
#include <stdio.h>

#define WIN_ERROR fprintf(stderr,"error code : %d , line %d in '%s'\n",GetLastError(), __LINE__, __FILE__);

static HANDLE hNamedPipe;

static BOOL connect_named_pipe(HANDLE *p){
	HANDLE handle;
	WaitNamedPipe(SERVER_PIPE, NMPWAIT_WAIT_FOREVER);
	handle = CreateFile(SERVER_PIPE, GENERIC_READ | GENERIC_WRITE, 0,
			NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE) {
		WIN_ERROR;
		return 0;
	}else{
		*p = handle;
		return 1;
	}
}

PluginGigasoAPI::PluginGigasoAPI(const PluginGigasoPtr& plugin, const FB::BrowserHostPtr& host) : m_plugin(plugin), m_host(host)
{
    registerMethod("search",      make_method(this, &PluginGigasoAPI::search));
    registerMethod("stat",      make_method(this, &PluginGigasoAPI::stat));

    registerMethod("shell_open",      make_method(this, &PluginGigasoAPI::shell_open));
	registerMethod("shell_edit",      make_method(this, &PluginGigasoAPI::shell_edit));
	registerMethod("shell_explore",      make_method(this, &PluginGigasoAPI::shell_explore));
	registerMethod("shell_find",      make_method(this, &PluginGigasoAPI::shell_find));
	registerMethod("shell_print",      make_method(this, &PluginGigasoAPI::shell_print));
	registerMethod("shell_default",      make_method(this, &PluginGigasoAPI::shell_default));
	registerMethod("shell2_prop",      make_method(this, &PluginGigasoAPI::shell2_prop));
	registerMethod("shell2_openas",      make_method(this, &PluginGigasoAPI::shell2_openas));
	registerMethod("shell2_default",      make_method(this, &PluginGigasoAPI::shell2_default));
	registerMethod("shell2",      make_method(this, &PluginGigasoAPI::shell2));
	registerMethod("copy_str",      make_method(this, &PluginGigasoAPI::copy_str));

    registerMethod("testEvent", make_method(this, &PluginGigasoAPI::testEvent));

    // Read-write property
    registerProperty("order",
                     make_property(this,
                        &PluginGigasoAPI::get_order,
                        &PluginGigasoAPI::set_order));
    registerProperty("file_type",
                     make_property(this,
                        &PluginGigasoAPI::get_file_type,
                        &PluginGigasoAPI::set_file_type));
    registerProperty("case",
                     make_property(this,
                        &PluginGigasoAPI::get_case,
                        &PluginGigasoAPI::set_case));
    // Read-only property
    registerProperty("version",
                     make_property(this,
                        &PluginGigasoAPI::get_version));
    
    
    registerEvent("onfired");    
	connect_named_pipe(&hNamedPipe);
	m_order=0;
	m_case=0;
	m_file_type=0;
}

PluginGigasoAPI::~PluginGigasoAPI(){
		CloseHandle(hNamedPipe);
}

PluginGigasoPtr PluginGigasoAPI::getPlugin(){
    PluginGigasoPtr plugin(m_plugin.lock());
    if (!plugin) {
        throw FB::script_error("The plugin is invalid");
    }
    return plugin;
}

int PluginGigasoAPI::get_order(){
    return m_order;
}
void PluginGigasoAPI::set_order(int val){
    m_order = val;
}
int PluginGigasoAPI::get_file_type(){
    return m_file_type;
}
void PluginGigasoAPI::set_file_type(int val){
    m_file_type = val;
}
bool PluginGigasoAPI::get_case(){
    return m_case;
}
void PluginGigasoAPI::set_case(bool val){
    m_case = val;
}

// Read-only property version
std::string PluginGigasoAPI::get_version()
{
    return "CURRENT_VERSION";
}

static int MAX_ROW = 1000;

FB::variant PluginGigasoAPI::search(const FB::variant& msg){
	SearchRequest req;
	SearchResponse resp;
	DWORD nRead, nWrite;
	memset(&req,0,sizeof(SearchRequest));
	req.from = 0;
	req.rows = MAX_ROW;
	req.env.order = m_order;
	req.env.case_sensitive = m_case;
	req.env.file_type = m_file_type;
	std::wstring s = msg.convert_cast<std::wstring>();
	wcscpy(req.str,s.c_str());
	if (!WriteFile(hNamedPipe, &req, sizeof(SearchRequest), &nWrite, NULL)) {
		WIN_ERROR;
		return "error";
	}
	if(ReadFile(hNamedPipe, &resp, sizeof(int), &nRead, NULL)  && resp.len>0){
		char buffer[MAX_RESPONSE_LEN];
		printf("%d,", resp.len);
		ReadFile(hNamedPipe, buffer, resp.len, &nRead, NULL);
		if(nRead!=resp.len){
			return "error";
		}
		std::string ret(buffer,resp.len) ;
		FB::variant var(ret);
		return var;
	}
	return msg;
}

FB::variant PluginGigasoAPI::stat(const FB::variant& msg){
	SearchRequest req;
	SearchResponse resp;
	DWORD nRead, nWrite;
	memset(&req,0,sizeof(SearchRequest));
	req.from = 0;
	req.rows = -1;
	req.env.order = m_order;
	req.env.case_sensitive = m_case;
	req.env.file_type = m_file_type;
	std::wstring s = msg.convert_cast<std::wstring>();
	wcscpy(req.str,s.c_str());
	if (!WriteFile(hNamedPipe, &req, sizeof(SearchRequest), &nWrite, NULL)) {
		WIN_ERROR;
		return "error";
	}
	if(ReadFile(hNamedPipe, &resp, sizeof(int), &nRead, NULL)  && resp.len>0){
		char buffer[MAX_RESPONSE_LEN];
		printf("%d,", resp.len);
		ReadFile(hNamedPipe, buffer, resp.len, &nRead, NULL);
		if(nRead!=resp.len){
			return "error";
		}
		std::string ret(buffer,resp.len) ;
		FB::variant var(ret);
		return var;
	}
	return msg;
}

void PluginGigasoAPI::testEvent(const FB::variant& var)
{
    FireEvent("onfired", FB::variant_list_of(var)(true)(1));
}

static int shell_exec(const FB::variant& msg, const wchar_t *verb){
	std::wstring s = msg.convert_cast<std::wstring>();
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    HINSTANCE ret = ShellExecuteW(NULL,verb,s.c_str(),NULL,NULL,SW_SHOWNORMAL);
	return (int)ret > 32;
}

static int shell2_exec(const FB::variant& msg, const wchar_t *verb){
	std::wstring s = msg.convert_cast<std::wstring>();
	SHELLEXECUTEINFO ShExecInfo ={0};
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_INVOKEIDLIST ;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = verb;
	ShExecInfo.lpFile = s.c_str();
	ShExecInfo.lpParameters = NULL; 
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_SHOW;
	ShExecInfo.hInstApp = NULL; 
	return ShellExecuteEx(&ShExecInfo);
}

FB::variant PluginGigasoAPI::shell_default(const FB::variant& msg){
	return shell_exec(msg,NULL);
}

FB::variant PluginGigasoAPI::shell_open(const FB::variant& msg){
	return shell_exec(msg,L"open");
}

FB::variant PluginGigasoAPI::shell_edit(const FB::variant& msg){
	return shell_exec(msg,L"edit");
}


FB::variant PluginGigasoAPI::shell_explore(const FB::variant& msg){
	return shell_exec(msg,L"explore");
}


FB::variant PluginGigasoAPI::shell_find(const FB::variant& msg){
	return shell_exec(msg,L"find");
}

FB::variant PluginGigasoAPI::shell_print(const FB::variant& msg){
	return shell_exec(msg,L"print");
}

FB::variant PluginGigasoAPI::shell2_prop(const FB::variant& msg){
	return shell2_exec(msg, L"properties");
}

FB::variant PluginGigasoAPI::shell2_openas(const FB::variant& msg){
	return shell2_exec(msg, L"openas");
}

FB::variant PluginGigasoAPI::shell2_default(const FB::variant& msg){
	return shell2_exec(msg, NULL);
}

FB::variant PluginGigasoAPI::shell2(const FB::variant& msg, const FB::variant& verb){
	std::wstring s = verb.convert_cast<std::wstring>();
	return shell2_exec(msg, s.c_str());
}

FB::variant PluginGigasoAPI::copy_str(const FB::variant& msg){
	std::wstring s = msg.convert_cast<std::wstring>();
	HGLOBAL hGlobal  = GlobalAlloc (GHND | GMEM_SHARE, (wcslen(s.c_str())+1)*sizeof(wchar_t));
	LPVOID pGlobal = GlobalLock (hGlobal) ;
	wcscpy ( (wchar_t *)pGlobal, s.c_str()) ;
	GlobalUnlock (hGlobal) ;
	if(OpenClipboard (NULL)){
		EmptyClipboard () ;
		SetClipboardData (CF_UNICODETEXT, hGlobal) ;
		CloseClipboard () ;
		return 1;
	}
	return 0;
}