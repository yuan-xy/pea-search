/**********************************************************\

  Auto-generated PluginGigasoAPI.cpp

\**********************************************************/

#include "JSObject.h"
#include "variant_list.h"
#include "DOM/Document.h"

#include "PluginGigasoAPI.h"
#include "DOM/Window.h"

#include "sharelib.h"
#include "common.h"
#include "history.h"
#include "win_icon.h"
#include <stdlib.h>   
#include <stdio.h>

static HANDLE hNamedPipe=NULL;

#define WIN_ERROR(host)  {\
	char buffer[1024];\
	sprintf(buffer,"error code : %d , line %d in '%s'\n",GetLastError(), __LINE__, __FILE__);\
	std::string s(buffer);\
	host->htmlLog(s);\
}

static BOOL connect_named_pipe(HANDLE *p, FB::BrowserHostPtr host){
	HANDLE handle;
	BOOL b = WaitNamedPipe(SERVER_PIPE, NMPWAIT_USE_DEFAULT_WAIT);
	if(!b){
		WIN_ERROR(host);
		return 0;
	}
	handle = CreateFile(SERVER_PIPE, GENERIC_READ | GENERIC_WRITE, 0,
			NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE) {
		WIN_ERROR(host);
		return 0;
	}else{
		*p = handle;
		return 1;
	}
}

static void close_named_pipe(){
		if(hNamedPipe!=NULL) CloseHandle(hNamedPipe);
		hNamedPipe=NULL;
}

bool PluginGigasoAPI::security_check(){
	std::string location = m_host->getDOMWindow()->getLocation();
	if(strncmp("http",location.c_str(),4)==0) return true;
	return false;
}

PluginGigasoAPI::PluginGigasoAPI(const PluginGigasoPtr& plugin, const FB::BrowserHostPtr& host) : m_plugin(plugin), m_host(host)
{
	if(security_check()) return;
	registerMethod("history",      make_method(this, &PluginGigasoAPI::history));
	registerMethod("his_thumb",      make_method(this, &PluginGigasoAPI::his_thumb));
	registerMethod("his_del",      make_method(this, &PluginGigasoAPI::his_del));
	registerMethod("his_pin",      make_method(this, &PluginGigasoAPI::his_pin));
	registerMethod("his_unpin",      make_method(this, &PluginGigasoAPI::his_unpin));
    registerMethod("search",      make_method(this, &PluginGigasoAPI::search));
	registerMethod("search_async",      make_method(this, &PluginGigasoAPI::search_async));
    registerMethod("stat",      make_method(this, &PluginGigasoAPI::stat));
    registerMethod("start_server",      make_method(this, &PluginGigasoAPI::start_server));
    registerMethod("save",      make_method(this, &PluginGigasoAPI::save));

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
    registerProperty("caze",
                     make_property(this,
                        &PluginGigasoAPI::get_caze,
                        &PluginGigasoAPI::set_caze));
    registerProperty("offline",
                     make_property(this,
                        &PluginGigasoAPI::get_offline,
                        &PluginGigasoAPI::set_offline));
    registerProperty("dire",
                     make_property(this,
                        &PluginGigasoAPI::get_dir,
                        &PluginGigasoAPI::set_dir));
    registerProperty("hotkey",
                     make_property(this,
                        &PluginGigasoAPI::get_hk,
                        &PluginGigasoAPI::set_hk));
    // Read-only property
    registerProperty("version",
                     make_property(this,
                        &PluginGigasoAPI::get_version));
    registerProperty("connected",
                     make_property(this,
                        &PluginGigasoAPI::get_connected));    

    registerEvent("onfired");    
	connect_named_pipe(&hNamedPipe,m_host);
	m_order=0;
	m_case=0;
	m_file_type=0;
	m_offline=false;
	//m_dir = std::wstring(L"c:\\");
	setPWD("npPluginGigaso.dll");
}

PluginGigasoAPI::~PluginGigasoAPI(){
		close_named_pipe();
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
bool PluginGigasoAPI::get_caze(){
    return m_case;
}
void PluginGigasoAPI::set_caze(bool val){
    m_case = val;
}
bool PluginGigasoAPI::get_offline(){
    return m_offline;
}
void PluginGigasoAPI::set_offline(bool val){
    m_offline = val;
}
bool PluginGigasoAPI::get_connected(){
    return hNamedPipe!=NULL;
}
std::wstring PluginGigasoAPI::get_dir(){
	return m_dir;
}
void PluginGigasoAPI::set_dir(std::wstring s){
	m_dir = s;
}

int PluginGigasoAPI::get_hk(){
    return get_hotkey();
}
void PluginGigasoAPI::set_hk(int val){
    set_hotkey(val);
	HWND wnd = FindWindow(ListenerWindowClass,NULL);
	if(wnd!=NULL) SendMessage(wnd,WM_SET_HOTKEY,NULL,NULL);
}

std::wstring PluginGigasoAPI::get_version(){
	wchar_t buffer[MAX_PATH];
	GetCurrentDirectory(MAX_PATH,buffer);
	std::wstring ret(buffer,wcslen(buffer)) ;
	return ret;
}

static int MAX_ROW = 1000;

FB::variant PluginGigasoAPI::query(const FB::variant& msg, int rows){
	if(hNamedPipe==NULL){
		if(!connect_named_pipe(&hNamedPipe,m_host)) return "error";
	}
	SearchRequest req;
	SearchResponse resp;
	DWORD nRead, nWrite;
	memset(&req,0,sizeof(SearchRequest));
	req.from = 0;
	req.rows = rows;
	req.env.order = m_order;
	req.env.case_sensitive = m_case;
	req.env.offline = m_offline? 1:0;
	req.env.file_type = m_file_type;
	req.env.path_len = m_dir.length();
	wcsncpy(req.env.path_name, m_dir.c_str(), MAX_PATH);
	std::wstring s = msg.convert_cast<std::wstring>();
	if(s.length()==0) return "";
	wcscpy(req.str,s.c_str());
	if (!WriteFile(hNamedPipe, &req, sizeof(SearchRequest), &nWrite, NULL)) {
		WIN_ERROR(m_host);
		close_named_pipe();
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

FB::variant PluginGigasoAPI::search(const FB::variant& msg){
	return query(msg,MAX_ROW);
}
FB::variant PluginGigasoAPI::stat(const FB::variant& msg){
	return query(msg,-1);
}

void PluginGigasoAPI::search_async_thread( const FB::variant& msg, FB::JSObjectPtr &callback ){
	FB::variant result = query(msg,MAX_ROW);
    callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(result));
}

bool PluginGigasoAPI::search_async(const FB::variant& msg, FB::JSObjectPtr &callback){
    boost::thread t(boost::bind(&PluginGigasoAPI::search_async_thread,
         this, msg, callback));
    return true;
}

void PluginGigasoAPI::testEvent(const FB::variant& var)
{
    FireEvent("onfired", FB::variant_list_of(var)(true)(1));
}

static int shell_exec(const FB::variant& msg, const wchar_t *verb){
	std::wstring s = msg.convert_cast<std::wstring>();
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    HINSTANCE h = ShellExecuteW(NULL,verb,s.c_str(),NULL,NULL,SW_SHOWNORMAL);
	int ret = (int)h > 32;
	if(ret){
		history_add(s.c_str());
		history_save();
	}
	return ret;
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
	BOOL ret = ShellExecuteEx(&ShExecInfo);
	if(ret){
		if(verb==NULL || wcscmp(L"delete",verb)!=0){
			history_add(s.c_str());
			history_save();
		}
	}
	return ret;
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


FB::variant PluginGigasoAPI::history(){
	wchar_t buffer[VIEW_HISTORY*MAX_PATH];
	int len;
	history_load();
	len = history_to_json(buffer);
	std::wstring ret(buffer,len) ;
	FB::variant var(ret);
	return var;
}


static int thumb_index;
static void gen_thumb(wchar_t *file, int pin, void *context){
	wchar_t thumb_name[16];
	wsprintf(thumb_name,L".\\web\\%d.jpg%c",thumb_index,L'\0');
	gen_icon_xlarge(file, thumb_name);
	thumb_index++;
}

void PluginGigasoAPI::his_thumb(){
	thumb_index = 0;
	history_load();
	HistoryIterator(gen_thumb,NULL);
}

bool PluginGigasoAPI::his_del(int i){
	history_delete(i);
	history_save();
	return true;
}

bool PluginGigasoAPI::his_pin(int i){
	history_pin(i);
	history_save();
	return true;
}

bool PluginGigasoAPI::his_unpin(int i){
	history_unpin(i);
	history_save();
	return true;
}

bool PluginGigasoAPI::start_server(){
	//WinExec("net stop gigaso",0);
	WinExec("net start gigaso",0);
	return true;
}

bool PluginGigasoAPI::save(const FB::variant& filename,const FB::variant& content){
	if(security_check()) return false;
	std::string name = filename.convert_cast<std::string>();
	std::wstring s = content.convert_cast<std::wstring>();
	const wchar_t *cs = s.c_str();
	FILE *fp;
	fp = fopen(name.c_str(), "w, ccs=UNICODE");
	if(fp==NULL) return false;
	size_t len = fwrite(cs,sizeof(wchar_t),wcslen(cs),fp);
	fclose(fp);
	if(len!=wcslen(cs)) return false;
	return true;
}