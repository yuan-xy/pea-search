#include <gtk/gtk.h>
#include <webkit/webkit.h>
#include "inspector.h"

WebKitWebView *
create_gtk_window_around_it(WebKitWebInspector *inspector,
                            WebKitWebView      *webview, void *data)
{
	GtkWidget *win;
	GtkWidget *view;
	printf("create_gtk_window_around_it\n");
	win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(win), "Inspector");
	gtk_window_set_default_size(GTK_WINDOW(win), 600, 400);
	view = webkit_web_view_new();
	gtk_container_add(GTK_CONTAINER(win), view);
	g_object_set_data(G_OBJECT(webview), "inspector-window", win);
	return WEBKIT_WEB_VIEW(view);
}

static gboolean
show_inspector_window(WebKitWebInspector *inspector,
                      GtkWidget          *webview)
{
	GtkWidget *win;
	printf("create_gtk_window_around_it\n");
	win = g_object_get_data(G_OBJECT(webview), "inspector-window");
	gtk_widget_show_all(win);
	return TRUE;
}

static 	WebKitWebInspector *inspector;
WebKitWebInspector * setup_inspector(GtkWidget *web_view){
	WebKitWebSettings *setting = webkit_web_view_get_settings (WEBKIT_WEB_VIEW(web_view));
	g_object_set (G_OBJECT(setting), "enable-developer-extras", TRUE, NULL);
	inspector = webkit_web_view_get_inspector (WEBKIT_WEB_VIEW(web_view));
	//webkit_web_inspector_show(inspector);
	g_signal_connect (G_OBJECT (inspector), "inspect-web-view", G_CALLBACK(create_gtk_window_around_it), NULL);
	g_signal_connect (G_OBJECT (inspector), "show-window", G_CALLBACK(show_inspector_window), web_view);
	/*	
		g_signal_connect (G_OBJECT (inspector), "notify::inspected-uri", G_CALLBACK(inspected_uri_changed_do_stuff), NULL);
	*/
	return inspector;
}
