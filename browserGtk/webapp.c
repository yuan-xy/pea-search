#include <gtk/gtk.h>
#include <stdlib.h>
#include <webkit/webkit.h>
#include <JavaScriptCore/JavaScript.h>
#include "js.h"

static void hello( GtkWidget *widget, gpointer   data ) {
	gtk_main_quit();
}

static void
load_commit_cb (WebKitWebView* page, WebKitWebFrame* frame, gpointer data)
{
    const gchar* uri = webkit_web_frame_get_uri(frame);
	printf("load %s done!\n",uri);
}

int main (int argc, char *argv[]){
	gchar cbuf[256];
	char *uri;
	gtk_init (&argc, &argv);
	GtkWidget *web_view = webkit_web_view_new ();
	GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (gtk_main_quit), NULL);
	//gtk_window_set_default_size(GTK_WINDOW (window), 680, 480);
	gtk_container_add (GTK_CONTAINER (window), web_view);
	WebKitWebSettings *setting = webkit_web_view_get_settings (WEBKIT_WEB_VIEW(web_view));
	g_object_set (G_OBJECT(setting), "enable-developer-extras", TRUE, NULL);
	if (argc < 2 && (uri = getenv("PWD")) != 0 && *uri == '/') {
		//sprintf(cbuf, "file://%s/text.html", uri);
		//uri = cbuf;
		uri = "/Users/ylt/Documents/gigaso/browser/web/search2.htm";
	}
	else
		uri = argv[1];
	webkit_web_view_open (WEBKIT_WEB_VIEW (web_view), uri);
	gtk_widget_grab_focus(GTK_WIDGET(web_view));
	gtk_widget_show_all (window);
	g_signal_connect(G_OBJECT (web_view), "window-object-cleared", G_CALLBACK(register_js), web_view);
	g_signal_connect(G_OBJECT (web_view), "load-committed", G_CALLBACK (load_commit_cb), web_view);
    
/*	
	WebKitWebInspector *inspector = webkit_web_view_get_inspector (WEBKIT_WEB_VIEW(web_view));
	webkit_web_inspector_show(inspector);
	g_signal_connect (G_OBJECT (inspector), "inspect-web-view", G_CALLBACK(create_gtk_window_around_it), NULL);
	g_signal_connect (G_OBJECT (inspector), "show-window", G_CALLBACK(show_inpector_window), NULL));
	g_signal_connect (G_OBJECT (inspector), "notify::inspected-uri", G_CALLBACK(inspected_uri_changed_do_stuff), NULL);
*/
	gtk_main ();
	return 0;
}
