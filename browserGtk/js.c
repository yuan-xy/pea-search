#include <gtk/gtk.h>
#include <stdlib.h>
#include <webkit/webkit.h>
#include <JavaScriptCore/JavaScript.h>

void foo_Initialize(JSContextRef ctx, JSObjectRef object)
{
}
void foo_Finalize(JSObjectRef object)
{
}
JSValueRef foo_GetVerbose(
           JSContextRef ctx,
           JSObjectRef  object,
           JSStringRef  propertyName,
           JSValueRef  *exception)
{
    // verbose is false
    return JSValueMakeBoolean(ctx, false);
}
JSValueRef foo_Print(JSContextRef ctx,
                       JSObjectRef function,
                       JSObjectRef thisObject,
                       size_t argumentCount,
                       const JSValueRef arguments[],
                       JSValueRef *exception)
{
 JSStringRef str = JSValueToStringCopy(ctx, arguments[0], exception);
 size_t size = JSStringGetMaximumUTF8CStringSize(str);
 char utf8[size];
 
 JSStringGetUTF8CString(str, utf8, size);
 
 fprintf(stderr, "utf8 = %s(%u)\n", utf8, size);
 return JSValueMakeNull(ctx);
}

// 类创建函数：
JSClassRef Foo_ClassCreate(JSContextRef ctx)
{
    static JSClassRef fooClass = NULL;
    if (fooClass) {
        // already created, just return
        return fooClass;
    }
 
   
    JSStaticFunction fooStaticFunctions[] = {
        { "print",           foo_Print,           kJSPropertyAttributeNone },
        { NULL, 0, 0 },
    };
   
   
    JSStaticFunction fooStaticValues[] = {
        { "Verbose",   foo_GetVerbose,  NULL,  kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
        { NULL, 0, 0, 0},
    };
   
   
    JSClassDefinition classdef = kJSClassDefinitionEmpty;
    classdef.className         = "Foo";
    classdef.initialize        = foo_Initialize;
    classdef.finalize          = foo_Finalize;
    classdef.staticValues      = fooStaticValues;
    classdef.staticFunctions   = fooStaticFunctions;
    return fooClass = JSClassCreate(&classdef);
}

void register_js(
            WebKitWebView  *wv,
            WebKitWebFrame *wf,
            gpointer        ctx,
            gpointer        arg3,
            gpointer        user_data)
{
    JSStringRef name = JSStringCreateWithUTF8CString("Foo");
    // Make the javascript object
    JSObjectRef obj = JSObjectMake(ctx, Foo_ClassCreate(ctx), NULL);
    // Set the property
    JSObjectSetProperty(ctx, JSContextGetGlobalObject(ctx), name, obj,kJSPropertyAttributeNone, NULL);
}
