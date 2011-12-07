#include <gtk/gtk.h>
#include <stdlib.h>
#include <webkit/webkit.h>
#include <JavaScriptCore/JavaScript.h>
#include "inspector.h"
#include "common.h"

static WebKitWebView  *webview;

static int order=0;
static int file_type=0;
static bool caze=false;
static bool offline=false;
static bool personal=false;
static int fontSize=12;

JSValueRef cef_get_order(JSContextRef ctx, JSObjectRef  object, JSStringRef  name, JSValueRef  *e){
	return JSValueMakeNumber(ctx, order);
}
bool cef_set_order(JSContextRef ctx, JSObjectRef object, JSStringRef name, JSValueRef value, JSValueRef* e){
	order = (int) JSValueToNumber(ctx, value, e);
	return true;
}

JSValueRef cef_get_caze(JSContextRef ctx, JSObjectRef  object, JSStringRef  name, JSValueRef  *e){
	return JSValueMakeBoolean(ctx, caze);
}
bool cef_set_caze(JSContextRef ctx, JSObjectRef object, JSStringRef name, JSValueRef value, JSValueRef* e){
	caze = (bool) JSValueToBoolean(ctx, value);
	return true;
}

JSValueRef cef_get_offline(JSContextRef ctx, JSObjectRef  object, JSStringRef  name, JSValueRef  *e){
	return JSValueMakeBoolean(ctx, offline);
}
bool cef_set_offline(JSContextRef ctx, JSObjectRef object, JSStringRef name, JSValueRef value, JSValueRef* e){
	offline = (bool) JSValueToBoolean(ctx, value);
	return true;
}

JSValueRef cef_get_personal(JSContextRef ctx, JSObjectRef  object, JSStringRef  name, JSValueRef  *e){
	return JSValueMakeBoolean(ctx, personal);
}
bool cef_set_personal(JSContextRef ctx, JSObjectRef object, JSStringRef name, JSValueRef value, JSValueRef* e){
	personal = (bool) JSValueToBoolean(ctx, value);
	return true;
}

JSValueRef cef_get_file_type(JSContextRef ctx, JSObjectRef  object, JSStringRef  name, JSValueRef  *e){
	return JSValueMakeNumber(ctx, file_type);
}
bool cef_set_file_type(JSContextRef ctx, JSObjectRef object, JSStringRef name, JSValueRef value, JSValueRef* e){
	file_type = (int) JSValueToNumber(ctx, value, e);
	return true;
}

JSValueRef cef_get_fontSize(JSContextRef ctx, JSObjectRef  object, JSStringRef  name, JSValueRef  *e){
	return JSValueMakeNumber(ctx, fontSize);
}
bool cef_set_fontSize(JSContextRef ctx, JSObjectRef object, JSStringRef name, JSValueRef value, JSValueRef* e){
	fontSize = (int) JSValueToNumber(ctx, value, e);
	return true;
}

#define GEN_CEF_PROP(x) \
JSValueRef cef_##x(JSContextRef ctx, JSObjectRef  object, JSStringRef  propertyName, JSValueRef  *exception){ \
	char buf[MAX_PATH]; \
    get_##x(buf); \
	JSStringRef s = JSStringCreateWithUTF8CString(buf); \
    return JSValueMakeString(ctx, s); \
} 
GEN_CEF_PROP(os)
GEN_CEF_PROP(cpu)
GEN_CEF_PROP(disk)
GEN_CEF_PROP(ver)
GEN_CEF_PROP(user)


JSValueRef cef_Print(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception){
 JSStringRef str = JSValueToStringCopy(ctx, arguments[0], exception);
 size_t size = JSStringGetMaximumUTF8CStringSize(str);
 char utf8[size];
 
 JSStringGetUTF8CString(str, utf8, size);
 
 fprintf(stderr, "utf8 = %s(%u)\n", utf8, size);
 return JSValueMakeNull(ctx);
}

void cef_devTool(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception){
	//可以通过右键访问devTool，但是直接编程访问不行。
	//JSEvaluateScript(ctx, JSStringCreateWithUTF8CString("alert('ok')"), NULL, NULL, 1, NULL);
}

JSClassRef Cef_ClassCreate(JSContextRef ctx){
    static JSClassRef cefClass = NULL;
    if (cefClass) {
        return cefClass;
    }
    JSStaticFunction cefStaticFunctions[] = {
        { "print",           cef_Print,           kJSPropertyAttributeNone },
        { "devTool",           cef_devTool,           kJSPropertyAttributeNone },
        { NULL, 0, 0 },
    };
    JSStaticValue cefStaticValues[] = {
		{ "order",   cef_get_order,  cef_set_order,  kJSPropertyAttributeDontDelete },	
		{ "file_type",   cef_get_file_type,  cef_set_file_type,  kJSPropertyAttributeDontDelete },	
		{ "caze",   cef_get_caze,  cef_set_caze,  kJSPropertyAttributeDontDelete },	
		{ "offline",   cef_get_offline,  cef_set_offline,  kJSPropertyAttributeDontDelete },	
		{ "personal",   cef_get_personal,  cef_set_personal,  kJSPropertyAttributeDontDelete },	
		{ "fontSize",   cef_get_fontSize,  cef_set_fontSize,  kJSPropertyAttributeDontDelete },			
		{ "os",   cef_os,  NULL,  kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
		{ "cpu",   cef_cpu,  NULL,  kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
		{ "disk",   cef_disk,  NULL,  kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
		{ "ver",   cef_ver,  NULL,  kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
		{ "user",   cef_user,  NULL,  kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
        { NULL, 0, 0, 0},
    };
    JSClassDefinition classdef = kJSClassDefinitionEmpty;
    classdef.className         = "Cef";
    classdef.staticValues      = cefStaticValues;
    classdef.staticFunctions   = cefStaticFunctions;
    return cefClass = JSClassCreate(&classdef);
}

void register_js(
            WebKitWebView  *wv,
            WebKitWebFrame *wf,
            gpointer        ctx,
            gpointer        arg3,
            gpointer        user_data)
{
	webview = wv;
    JSStringRef name = JSStringCreateWithUTF8CString("cef");
    // Make the javascript object
    JSObjectRef obj = JSObjectMake(ctx, Cef_ClassCreate(ctx), NULL);
    // Set the property
    JSObjectSetProperty(ctx, JSContextGetGlobalObject(ctx), name, obj,kJSPropertyAttributeNone, NULL);
}
