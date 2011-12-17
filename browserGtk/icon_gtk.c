#include "icon_gtk.h"
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

static void gen_icon_image0(GIcon *icon, char *imagename, int size){
        GdkPixbuf *app_icon;
        GtkIconTheme *icontheme;
		char *icon_name = gicon_to_string(icon);
        icontheme = gtk_icon_theme_get_default();
        app_icon = gtk_icon_theme_load_icon(icontheme, icon_name, size, 0, NULL);
		gdk_pixbuf_savev(app_icon,imagename,"png",NULL,NULL,NULL);
}

void gen_icon_image(char *filename, char *imagename){
        GFile *file;
		GFileInfo *file_info;
		file = g_file_new_for_uri(filename);
		file_info = g_file_query_info(file,G_FILE_ATTRIBUTE_STANDARD_ICON, G_FILE_QUERY_INFO_NONE, NULL, NULL);
		if (file_info) {
			GIcon *icon = g_file_info_get_icon(file_info);
			gen_icon_image0(icon,imagename,80);
		    g_object_unref(file_info);
		}
		g_object_unref(file);
}

