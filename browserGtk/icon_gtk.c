#include "icon_gtk.h"
#include "base64/base64.h"
#include <gtk/gtk.h>

static char *
gicon_to_string (GIcon *icon)
{
  GFile *file;
  const char *const *names;

  if (G_IS_FILE_ICON (icon))
    {
      file = g_file_icon_get_file (G_FILE_ICON (icon));
      if (file)
        return g_file_get_path (file);
    }
  else if (G_IS_THEMED_ICON (icon))
    {
      names = g_themed_icon_get_names (G_THEMED_ICON (icon));
      if (names)
        return g_strdup (names[0]);
    }
  else if (G_IS_EMBLEMED_ICON (icon))
    {
      GIcon *base;

      base = g_emblemed_icon_get_icon (G_EMBLEMED_ICON (icon));

      return gicon_to_string (base);
    }

  return NULL;
}

static GIcon *
get_icon (GFile *file)
{
  GFileInfo *info;
  GIcon *icon;

  icon = NULL;
  info = g_file_query_info (file, G_FILE_ATTRIBUTE_STANDARD_ICON, 0, NULL, NULL);
  if (info)
    {
      icon = g_file_info_get_icon (info);
      if (icon)
        g_object_ref (icon);
      g_object_unref (info);
    }

  return icon;
}

static GIcon *
get_icon_from_file (char *filename){
	GFile *file;
	file = g_file_new_for_uri(filename);
	GIcon *ret = get_icon(file);
	g_object_unref(file);
	return ret;
}

static GdkPixbuf *get_pixbuf(GIcon *icon, int size){
	GdkPixbuf *ret;
    GtkIconTheme *icontheme;
	char *icon_name = gicon_to_string(icon);
    icontheme = gtk_icon_theme_get_default();
    ret = gtk_icon_theme_load_icon(icontheme, icon_name, size, 0, NULL);
	g_object_ref (ret);
	return ret;
}

void gen_icon_image(char *filename, char *imagename){
	GIcon *icon = get_icon_from_file(filename);
	GdkPixbuf *pixbuf = get_pixbuf(icon,80);
	gdk_pixbuf_savev(pixbuf,imagename,"png",NULL,NULL,NULL);
}

void gen_icon_base64_url(char *filename, char *buffer, int *buffer_size){
	gsize gbuffer_size;
	char *buf = NULL;
	GIcon *icon = get_icon_from_file(filename);
	GdkPixbuf *pixbuf = get_pixbuf(icon,100);
	gdk_pixbuf_save_to_buffer(pixbuf,&buf,&gbuffer_size,"png",NULL,NULL);
	//printf("%s\n",buf);
	{
		char *p = stpcpy(buffer,"data:image/png;base64,");
		base64_encode_block (p, buffer_size, buf, gbuffer_size);
	}
}

