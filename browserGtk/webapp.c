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

static void hello( GtkWidget *widget, gpointer   data ) {
	gtk_main_quit();
}

int main (int argc, char *argv[])
{
	GtkWidget *button;
	GtkWidget *box;
	gchar cbuf[256];
	char *uri;

	gtk_init (&argc, &argv);

	button = gtk_button_new_with_label ("Press *me*");
	box = gtk_hbox_new (FALSE, 0);

	GtkWidget *web_view = webkit_web_view_new ();

	GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (hello), NULL);
	gtk_window_set_default_size(GTK_WINDOW (window), 680, 480);

	gtk_container_add (GTK_CONTAINER (window), web_view);
	//gtk_box_pack_start (GTK_BOX(box), button, TRUE, TRUE, 0);

	//gtk_box_pack_start (GTK_BOX(box), web_view, TRUE, TRUE, 0);

	gtk_widget_show_all (window);

	if (argc < 2 && (uri = getenv("PWD")) != 0 && *uri == '/') {
		//sprintf(cbuf, "file://%s/text.html", uri);
		//uri = cbuf;
		uri = "/Users/ylt/Documents/gigaso/browser/web/search2.htm";
	}
	else
		uri = argv[1];
	webkit_web_view_open (WEBKIT_WEB_VIEW (web_view), uri);
	g_signal_connect(G_OBJECT (web_view), "window-object-cleared", G_CALLBACK(register_js), web_view);

	gtk_main ();
	return 0;
}
