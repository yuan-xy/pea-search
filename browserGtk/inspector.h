
#ifndef INSPECTOR_ENV_H_
#define INSPECTOR_ENV_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <gtk/gtk.h>
#include <webkit/webkit.h>

extern WebKitWebInspector * setup_inspector(GtkWidget *web_view);



#ifdef __cplusplus
}
#endif

#endif  // INSPECTOR_ENV_H_