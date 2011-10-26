#include "env.h"
#include "cef_plugin.h"
#include "client_handler.h"
#include "sharelib.h"
#include "common.h"
#include "history.h"
#include "win_icon.h"
#include "desktop_exec.h"
#include "string_util.h"
#include "explorer++/FileOperations.h"

extern CefRefPtr<ClientHandler> g_handler;

static int MAX_ROW = 1000;
static HANDLE hNamedPipe=NULL;

static int m_order=0;
static int m_file_type=0;
static bool m_case=false;
static bool m_offline=false;
static CefString m_dir;

#define WIN_ERROR(host)  {\
	char buffer[1024];\
	sprintf(buffer,"error code : %d , line %d in '%s'\n",GetLastError(), __LINE__, __FILE__);\
}

BOOL connect_named_pipe(){
	BOOL b = WaitNamedPipe(SERVER_PIPE, NMPWAIT_USE_DEFAULT_WAIT);
	if(!b){
		WIN_ERROR(host);
		return 0;
	}
	hNamedPipe = CreateFile(SERVER_PIPE, GENERIC_READ | GENERIC_WRITE, 0,
			NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hNamedPipe == INVALID_HANDLE_VALUE) {
		WIN_ERROR(host);
		return 0;
	}else{
		return 1;
	}
}

void close_named_pipe(){
		if(hNamedPipe!=NULL) CloseHandle(hNamedPipe);
		hNamedPipe=NULL;
}

static CefString query(CefString msg, int rows){
	if(hNamedPipe==NULL){
		if(!connect_named_pipe()) return "error";
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
	if(req.env.path_len>0) wcsncpy(req.env.path_name, m_dir.c_str(), MAX_PATH);
	{
		DWORD size=MAX_PATH;
		GetUserName(req.env.user_name, &size);
	}
	std::wstring s = msg.ToWString();
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
		CefString ret2 = CefString("");
		ret2.FromString(ret);
		return ret2;
	}
	return "error";
}

static CefString search(CefString msg){
	return query(msg,MAX_ROW);
}
static CefString stat(CefString msg){
	return query(msg,-1);
}

static bool shell2_exec0(const wchar_t *file,const wchar_t *verb){
	SHELLEXECUTEINFO ShExecInfo ={0};
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_INVOKEIDLIST ;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = verb;
	ShExecInfo.lpFile = file;
	ShExecInfo.lpParameters = NULL; 
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_SHOW;
	ShExecInfo.hInstApp = NULL; 
	bool ret = ShellExecuteEx(&ShExecInfo);
	if(ret){
		if(verb==NULL || wcscmp(L"delete",verb)!=0){
			if( history_add(file) ) history_save();
		}
	}
	return ret;
}
static bool shell2_exec(std::wstring s, const wchar_t *verb){
	return shell2_exec0(s.c_str(),verb);
}

static bool shellExplore(CefString msg){
	char buffer[MAX_PATH*2];
	std::string as = WStringToString(msg.ToWString());
	_snprintf(buffer,MAX_PATH*2,"explorer /n, /Select, \"%s\"\0",as.c_str());
	return WinExec(buffer, SW_NORMAL) > 31;
}

static bool shellDefault(CefString msg){
	std::wstring str = msg.ToWString();
	const wchar_t *c = str.c_str();
	if(*c==L'\\'){
		BOOL ret = exec_desktop(c);
		if(ret){
			if( history_add(c) ) history_save();
		}
		return ret;
	}else{
		return shell2_exec(str, NULL);
	}
}

static bool shell2(CefString msg, CefString verb){
	std::wstring s = verb.ToWString();
	return shell2_exec(msg.ToWString(), s.c_str());
}

static bool copy_str(CefString msg){
	std::wstring s = msg.ToWString();
	HGLOBAL hGlobal  = GlobalAlloc (GHND | GMEM_SHARE, (wcslen(s.c_str())+1)*sizeof(wchar_t));
	LPVOID pGlobal = GlobalLock (hGlobal) ;
	wcscpy ( (wchar_t *)pGlobal, s.c_str()) ;
	GlobalUnlock (hGlobal) ;
	if(OpenClipboard (NULL)){
		EmptyClipboard () ;
		SetClipboardData (CF_UNICODETEXT, hGlobal) ;
		CloseClipboard () ;
		return true;
	}
	return false;
}

static bool save(CefString filename,CefString content){
	std::string name = filename.ToString();
	std::wstring s = content.ToWString();
	const wchar_t *cs = s.c_str();
	FILE *fp;
	fp = fopen(name.c_str(), "w, ccs=UNICODE");
	if(fp==NULL) return false;
	size_t len = fwrite(cs,sizeof(wchar_t),wcslen(cs),fp);
	fclose(fp);
	if(len!=wcslen(cs)) return false;
	return true;
}

static bool start_server(){
	return WinExec("net start PeaSearch",0) > 31;
}

static CefString history(){
	wchar_t buffer[VIEW_HISTORY*MAX_PATH];
	int len;
	history_load();
	len = history_to_json(buffer);
	std::wstring ret(buffer,len) ;
	CefString ret2 = CefString("");
	ret2.FromWString(ret);
	return ret2;
}

static HWND find_listener_window(){
	wchar_t fbuffer[128];
	DWORD size=128;
	GetUserName(fbuffer, &size);
	return FindWindow(ListenerWindowClass,fbuffer);
}

#define CEF_METHOD(method_name,type) if(name == #method_name ){ \
      if(arguments.size() != 1 || !arguments[0]->IsString()){ \
        return false; \
      }else{ \
		retval = CefV8Value::Create##type(method_name(arguments[0]->GetStringValue())); \
		return true; \
	  } \
}

// Implementation of the V8 handler class for the "cef.plugin" extension.
class PluginHandler : public CefV8Handler{
public:
  PluginHandler(){}
  virtual ~PluginHandler() {}

  virtual bool Execute(const CefString& name,
                       CefRefPtr<CefV8Value> object,
                       const CefV8ValueList& arguments,
                       CefRefPtr<CefV8Value>& retval,
                       CefString& exception)
  {
	CEF_METHOD(search,String)
	CEF_METHOD(stat,String)
	CEF_METHOD(shellExplore,Bool)
	CEF_METHOD(shellDefault,Bool)
	CEF_METHOD(copy_str,Bool)     

    if(name == "shell2"){
      if(arguments.size() != 2 || !arguments[0]->IsString() || !arguments[1]->IsString())
        return false;
	  bool b = shell2(arguments[0]->GetStringValue(),arguments[1]->GetStringValue());
	  retval = CefV8Value::CreateBool(b);
      return true;
    }
    if(name == "save"){
      if(arguments.size() != 2 || !arguments[0]->IsString() || !arguments[1]->IsString())
        return false;
	  bool b = save(arguments[0]->GetStringValue(),arguments[1]->GetStringValue());
	  retval = CefV8Value::CreateBool(b);
      return true;
    }
    if(name == "start_server"){
	  retval = CefV8Value::CreateBool(start_server());
      return true;
    }
    if(name == "history"){
	  retval = CefV8Value::CreateString(history());
      return true;
    }
    if(name == "hisDel"){
      if(arguments.size() != 1 || !arguments[0]->IsInt()) return false;
      history_delete(arguments[0]->GetIntValue());
	  history_save();
      retval = CefV8Value::CreateBool(true);
      return true;
    }
    if(name == "hisPin"){
      if(arguments.size() != 1 || !arguments[0]->IsInt()) return false;
      history_pin(arguments[0]->GetIntValue());
	  history_save();
      retval = CefV8Value::CreateBool(true);
      return true;
    }
    if(name == "hisUnpin"){
      if(arguments.size() != 1 || !arguments[0]->IsInt()) return false;
      history_unpin(arguments[0]->GetIntValue());
	  history_save();
      retval = CefV8Value::CreateBool(true);
      return true;
    }

#define PARSE_FILE \
      if(arguments.size() != 1 || !arguments[0]->IsString()) return false;\
	  std::wstring ssss = arguments[0]->GetStringValue().ToWString();\
	  wchar_t *files = (wchar_t *)ssss.c_str();\
	  int len = (int)wcslen(files);\
	  if(*(files+len-1)!=L'|') return false;\
	  for(int i=0;i<len;i++){\
		  if(*(files+i)==L'|') *(files+i)=L'\0';\
	  }

    if(name == "batch_copy"){
	  PARSE_FILE;
	  IDataObject *pClipboardDataObject;
	  CopyFiles(files,(len+1) * sizeof(wchar_t),&pClipboardDataObject);
	  retval = CefV8Value::CreateBool(true);
      return true;
    }
    if(name == "batch_cut"){
	  PARSE_FILE;
	  IDataObject *pClipboardDataObject;
	  CutFiles(files,(len+1) * sizeof(wchar_t),&pClipboardDataObject);
	  retval = CefV8Value::CreateBool(true);
      return true;
    }
    if(name == "batch_delete"){
	  PARSE_FILE;
	  DeleteFilesToRecycleBin(NULL,files);
	  retval = CefV8Value::CreateBool(true);
      return true;
    }
    if(name == "batch_open"){
	  PARSE_FILE;
	  wchar_t *pfile=files;
	  do{
		shell2_exec0(pfile,L"open");
		while((pfile-files<len) && *pfile++ != L'\0');
	  }while(pfile-files<len);
	  retval = CefV8Value::CreateBool(true);
      return true;
    }
    if(name == "get_order"){
      retval = CefV8Value::CreateInt(m_order);
      return true;
    }
    else if(name == "set_order"){
      if(arguments.size() != 1)
        return false;
      m_order = arguments[0]->GetIntValue();
      return true;
    }
	else if(name == "get_offline"){
      retval = CefV8Value::CreateBool(m_offline);
      return true;
    }
	else if(name == "set_offline"){
      if(arguments.size() != 1 || !arguments[0]->IsBool())
        return false;
      m_offline = arguments[0]->GetBoolValue();
      return true;
    }
    else if(name == "get_file_type"){
      retval = CefV8Value::CreateInt(m_file_type);
      return true;
	}
    else if(name == "set_file_type"){
      if(arguments.size() != 1)
        return false;
      m_file_type = arguments[0]->GetIntValue();
      return true;
    }
    else if(name == "get_dire"){
      retval = CefV8Value::CreateString(m_dir);
      return true;
    }
    else if(name == "set_dire"){
      if(arguments.size() != 1 || !arguments[0]->IsString())
        return false;
      m_dir = arguments[0]->GetStringValue();
      return true;
    }
    else if(name == "get_caze"){
      retval = CefV8Value::CreateBool(m_case);
      return true;
    }
    else if(name == "set_caze"){
      if(arguments.size() != 1 || !arguments[0]->IsBool())
        return false;
      m_case = arguments[0]->GetBoolValue();
      return true;
    }
    else if(name == "get_hotkey"){
	  HWND wnd = find_listener_window();
      if(wnd==NULL) return false;
	  int hotkey = SendMessage(wnd,WM_GET_HOTKEY,NULL,NULL);
      retval = CefV8Value::CreateInt(hotkey);
      return true;
    }
    else if(name == "set_hotkey"){
      if(arguments.size() != 1) return false;
      HWND wnd = find_listener_window();
      if(wnd==NULL) return false;
	  SendMessage(wnd,WM_SET_HOTKEY,arguments[0]->GetIntValue(),NULL);
      return true;
    }
    return false;
  }

  IMPLEMENT_REFCOUNTING(PluginHandler);
};

/*
#include "explorer++/Buffer.h"
static void aaaa(){
  IDataObject *pClipboardDataObject;
  IBufferManager	*pBufferManager = NULL;
  TCHAR			*szFileNameList = NULL;
	DWORD			dwBufSize;
	HRESULT			hr;
  pBufferManager = new CBufferManager();
  pBufferManager->WriteListEntry(L"c:\\boot.ini");
  pBufferManager->WriteListEntry(L"e:\\test.cpp");
  pBufferManager->QueryBufferSize(&dwBufSize);
  szFileNameList = (TCHAR *)malloc(dwBufSize * sizeof(TCHAR));
  pBufferManager->QueryBuffer(szFileNameList,dwBufSize);
  CopyFiles(szFileNameList,dwBufSize * sizeof(TCHAR),&pClipboardDataObject);
  CopyFiles(L"c:\\boot.ini\0d:\\bootmgr\0",24 * sizeof(TCHAR),&pClipboardDataObject);
}
*/

void InitPlugin(){
  std::string code = "var plugin;"
    "if (!cef)"
    "  cef = {};"
    "if (!cef.plugin)"
    "  cef.plugin = {};"
    "(function() {"
    "  cef.plugin.__defineGetter__('order', function() {"
    "    native function get_order();"
    "    return get_order();"
    "  });"
    "  cef.plugin.__defineSetter__('order', function(b) {"
    "    native function set_order();"
    "    set_order(b);"
    "  });"
    "  cef.plugin.__defineGetter__('offline', function() {"
    "    native function get_offline();"
    "    return get_offline();"
    "  });"
    "  cef.plugin.__defineSetter__('offline', function(b) {"
    "    native function set_offline();"
    "    set_offline(b);"
    "  });"
    "  cef.plugin.__defineGetter__('file_type', function() {"
    "    native function get_file_type();"
    "    return get_file_type();"
    "  });"
    "  cef.plugin.__defineSetter__('file_type', function(b) {"
    "    native function set_file_type();"
    "    set_file_type(b);"
    "  });"
    "  cef.plugin.__defineGetter__('caze', function() {"
    "    native function get_caze();"
    "    return get_caze();"
    "  });"
    "  cef.plugin.__defineSetter__('caze', function(b) {"
    "    native function set_caze();"
    "    set_caze(b);"
    "  });"
    "  cef.plugin.__defineGetter__('dire', function() {"
    "    native function get_dire();"
    "    return get_dire();"
    "  });"
    "  cef.plugin.__defineSetter__('dire', function(b) {"
    "    native function set_dire();"
    "    set_dire(b);"
    "  });"
    "  cef.plugin.__defineGetter__('hotkey', function() {"
    "    native function get_hotkey();"
    "    return get_hotkey();"
    "  });"
    "  cef.plugin.__defineSetter__('hotkey', function(b) {"
    "    native function set_hotkey();"
    "    set_hotkey(b);"
    "  });"
	"  cef.plugin.search = function(b) {"
    "    native function search();"
    "    return search(b);"
    "  };"
	"  cef.plugin.stat = function(b) {"
    "    native function stat();"
    "    return stat(b);"
    "  };"
	"  cef.plugin.shellExplore = function(b) {"
    "    native function shellExplore();"
    "    return shellExplore(b);"
    "  };"
	"  cef.plugin.shellDefault = function(b) {"
    "    native function shellDefault();"
    "    return shellDefault(b);"
    "  };"
	"  cef.plugin.shell2 = function(b,b2) {"
    "    native function shell2();"
    "    return shell2(b,b2);"
    "  };"
	"  cef.plugin.save = function(b,b2) {"
    "    native function save();"
    "    return save(b,b2);"
    "  };"
	"  cef.plugin.copy_str = function(b) {"
    "    native function copy_str();"
    "    return copy_str(b);"
    "  };"
	"  cef.plugin.start_server = function() {"
    "    native function start_server();"
    "    return start_server();"
    "  };"
	"  cef.plugin.history = function() {"
    "    native function history();"
    "    return history();"
    "  };"
	"  cef.plugin.hisDel = function(b) {"
    "    native function hisDel();"
    "    return hisDel(b);"
    "  };"
	"  cef.plugin.hisPin = function(b) {"
    "    native function hisPin();"
    "    return hisPin(b);"
    "  };"
	"  cef.plugin.hisUnpin = function(b) {"
    "    native function hisUnpin();"
    "    return hisUnpin(b);"
    "  };"
	"  cef.plugin.batch_open = function(b) {"
    "    native function batch_open();"
    "    return batch_open(b);"
    "  };"
	"  cef.plugin.batch_copy = function(b) {"
    "    native function batch_copy();"
    "    return batch_copy(b);"
    "  };"
	"  cef.plugin.batch_cut = function(b) {"
    "    native function batch_cut();"
    "    return batch_cut(b);"
    "  };"
	"  cef.plugin.batch_delete = function(b) {"
    "    native function batch_delete();"
    "    return batch_delete(b);"
    "  };"
	"})();";
  CefRegisterExtension("v8/gigaso.plugin", code, new PluginHandler());
}
