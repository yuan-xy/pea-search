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


///////////////////////////////////////////////////////////////////////////////
/// @fn PluginGigasoAPI::PluginGigasoAPI(const PluginGigasoPtr& plugin, const FB::BrowserHostPtr host)
///
/// @brief  Constructor for your JSAPI object.  You should register your methods, properties, and events
///         that should be accessible to Javascript from here.
///
/// @see FB::JSAPIAuto::registerMethod
/// @see FB::JSAPIAuto::registerProperty
/// @see FB::JSAPIAuto::registerEvent
///////////////////////////////////////////////////////////////////////////////
PluginGigasoAPI::PluginGigasoAPI(const PluginGigasoPtr& plugin, const FB::BrowserHostPtr& host) : m_plugin(plugin), m_host(host)
{
    registerMethod("echo",      make_method(this, &PluginGigasoAPI::echo));
    registerMethod("testEvent", make_method(this, &PluginGigasoAPI::testEvent));

    // Read-write property
    registerProperty("testString",
                     make_property(this,
                        &PluginGigasoAPI::get_testString,
                        &PluginGigasoAPI::set_testString));

    // Read-only property
    registerProperty("version",
                     make_property(this,
                        &PluginGigasoAPI::get_version));
    
    
    registerEvent("onfired");    
	connect_named_pipe(&hNamedPipe);
}

///////////////////////////////////////////////////////////////////////////////
/// @fn PluginGigasoAPI::~PluginGigasoAPI()
///
/// @brief  Destructor.  Remember that this object will not be released until
///         the browser is done with it; this will almost definitely be after
///         the plugin is released.
///////////////////////////////////////////////////////////////////////////////
PluginGigasoAPI::~PluginGigasoAPI()
{
		CloseHandle(hNamedPipe);
}

///////////////////////////////////////////////////////////////////////////////
/// @fn PluginGigasoPtr PluginGigasoAPI::getPlugin()
///
/// @brief  Gets a reference to the plugin that was passed in when the object
///         was created.  If the plugin has already been released then this
///         will throw a FB::script_error that will be translated into a
///         javascript exception in the page.
///////////////////////////////////////////////////////////////////////////////
PluginGigasoPtr PluginGigasoAPI::getPlugin()
{
    PluginGigasoPtr plugin(m_plugin.lock());
    if (!plugin) {
        throw FB::script_error("The plugin is invalid");
    }
    return plugin;
}



// Read/Write property testString
std::string PluginGigasoAPI::get_testString()
{
    return m_testString;
}
void PluginGigasoAPI::set_testString(const std::string& val)
{
    m_testString = val;
}

// Read-only property version
std::string PluginGigasoAPI::get_version()
{
    return "CURRENT_VERSION";
}

// Method echo
FB::variant PluginGigasoAPI::echo(const FB::variant& msg)
{
	SearchRequest req;
	SearchResponse resp;
	DWORD nRead, nWrite;
	memset(&req,0,sizeof(SearchRequest));
	req.from = 0;
	req.len = 10;
	std::wstring s = msg.convert_cast<std::wstring>();
	wcscpy(req.str,s.c_str());
	if(wcsncmp(L"er",s.c_str(),2)==0) return "error";
			if (!WriteFile(hNamedPipe, &req, sizeof(SearchRequest), &nWrite, NULL)) {
				WIN_ERROR;
				return "error";
			}
			if(ReadFile(hNamedPipe, &resp, sizeof(int), &nRead, NULL)  && resp.len>0){
				char buffer[40960];
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

