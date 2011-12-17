#include "icon_gtk.h"
#include <gtk/gtk.h>

int main (int argc, char *argv[]){
	char buffer[81920];
	int len;
	gtk_init (&argc, &argv);
	//gen_icon_image("file:///home/ylt/a.c","test.png");
	gen_icon_base64_url("file:///home/ylt/a.c",buffer,&len);
	printf("%s\n",buffer);
	return 0;
}