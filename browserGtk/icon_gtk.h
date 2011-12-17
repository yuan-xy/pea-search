
#ifndef ICON_GTK_ENV_H_
#define ICON_GTK_ENV_H_

#ifdef __cplusplus
extern "C" {
#endif


extern void gen_icon_image(char *filename, char *imagename);

extern void gen_icon_base64_url(char *filename, char *buffer, int *buffer_size);



#ifdef __cplusplus
}
#endif

#endif  // ICON_GTK_ENV_H_