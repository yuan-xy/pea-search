#include "icon_gtk.h"
#include <gtk/gtk.h>

int main (int argc, char *argv[]){
	gtk_init (&argc, &argv);
	gen_icon_image("file:///home/ylt/a.c","test.png");
	return 0;
}