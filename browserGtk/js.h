#include <gtk/gtk.h>
#include <stdlib.h>
#include <webkit/webkit.h>
#include <JavaScriptCore/JavaScript.h>


extern void register_js(
            WebKitWebView  *wv,
            WebKitWebFrame *wf,
            gpointer        ctx,
            gpointer        arg3,
			gpointer        user_data);

extern void load_config ();
