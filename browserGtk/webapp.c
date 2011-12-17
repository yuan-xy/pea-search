#include <gtk/gtk.h>
#include <stdlib.h>
#include <webkit/webkit.h>
#include <JavaScriptCore/JavaScript.h>
#include "js_gtk.h"
#include "inspector.h"
#include "unix_domain_client.h"

int	sockfd;

static void hello( GtkWidget *widget, gpointer   data ) {
	gtk_main_quit();
}

static void load_finished_cb (WebKitWebView* page, WebKitWebFrame* frame, gpointer data){
    const gchar* uri = webkit_web_frame_get_uri(frame);
	printf("load %s finished!\n",uri);
	JSGlobalContextRef context = webkit_web_frame_get_global_context(frame);
	eval_js(context,"init_dir('')");
}

int main (int argc, char *argv[]){
	gchar cbuf[256];
	char *uri;
	gtk_init (&argc, &argv);
	GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	GtkWidget *web_view = webkit_web_view_new ();
	//gtk_window_set_icon_from_file(window,"logo.png",NULL);
	gtk_window_set_default_icon_from_file("logo.png",NULL);
	g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (gtk_main_quit), NULL);
	gtk_window_set_default_size(GTK_WINDOW (window), 1000, 768);
	gtk_container_add (GTK_CONTAINER (window), web_view);
	gtk_widget_show_all (window);
	load_config ();
	if (argc < 2 && (uri = getenv("PWD")) != 0 && *uri == '/') {
		//sprintf(cbuf, "file://%s/text.html", uri);
		//uri = cbuf;
		uri = "/Users/ylt/Documents/gigaso/browser/web/search2.htm";
	}
	else
		uri = argv[1];
	webkit_web_view_open (WEBKIT_WEB_VIEW (web_view), uri);
	gtk_widget_grab_focus(GTK_WIDGET(web_view));
	g_signal_connect(G_OBJECT (web_view), "window-object-cleared", G_CALLBACK(register_js), web_view);
	//g_signal_connect(G_OBJECT (web_view), "load-committed", G_CALLBACK (load_commit_cb), web_view);
	g_signal_connect(G_OBJECT(web_view), "load-finished", G_CALLBACK(load_finished_cb), web_view);
	setup_inspector(web_view);
	connect_unix_socket(&sockfd);
	gtk_main ();
	return 0;
}
