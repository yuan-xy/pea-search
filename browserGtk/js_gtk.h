
#ifndef JS_GTK_ENV_H_
#define JS_GTK_ENV_H_

#ifdef __cplusplus
extern "C" {
#endif

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

extern void eval_js(JSContextRef ctx, char* script);

#ifdef __cplusplus
}
#endif

#endif  // JS_GTK_ENV_H_
