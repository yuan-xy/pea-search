/**********************************************************\

  Auto-generated PluginGigasoAPI.h

\**********************************************************/

#include <string>
#include <sstream>
#include <boost/weak_ptr.hpp>
#include "JSAPIAuto.h"
#include "BrowserHost.h"
#include "PluginGigaso.h"

#ifndef H_PluginGigasoAPI
#define H_PluginGigasoAPI

class PluginGigasoAPI : public FB::JSAPIAuto
{
public:
    PluginGigasoAPI(const PluginGigasoPtr& plugin, const FB::BrowserHostPtr& host);
    virtual ~PluginGigasoAPI();

    PluginGigasoPtr getPlugin();

    // Read/Write property ${PROPERTY.ident}
    std::string get_testString();
    void set_testString(const std::string& val);

    // Read-only property ${PROPERTY.ident}
    std::string get_version();

    FB::variant search(const FB::variant& msg);
    FB::variant shell_open(const FB::variant& msg);
    FB::variant shell_edit(const FB::variant& msg);
    FB::variant shell_explore(const FB::variant& msg);
    FB::variant shell_find(const FB::variant& msg);
    FB::variant shell_print(const FB::variant& msg);
    FB::variant shell_default(const FB::variant& msg);
    FB::variant shell2_prop(const FB::variant& msg);
    FB::variant shell2_openas(const FB::variant& msg);
    FB::variant shell2_default(const FB::variant& msg);
    FB::variant shell2(const FB::variant& msg, const FB::variant& verb);
	FB::variant copy_str(const FB::variant& msg);
    // Method test-event
    void testEvent(const FB::variant& s);

private:
    PluginGigasoWeakPtr m_plugin;
    FB::BrowserHostPtr m_host;

    std::string m_testString;
};

#endif // H_PluginGigasoAPI

