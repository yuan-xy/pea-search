#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_WIN_ICON_H_
#define FILE_SEARCH_WIN_ICON_H_

#include "env.h"

#define MAX_HISTORY 20

/**
* 根据文件szFileName的类型得到该文件的SHIL_EXTRALARGE类型的图标，并保存到文件iconFileName中。
*/
extern void gen_icon_xlarge(LPCTSTR szFileName, LPCTSTR iconFileName);

/**
* 根据文件szFileName的类型得到该文件的16*16小图标，并保存到文件iconFileName中。
*/
extern void gen_icon_small(LPCTSTR szFileName, LPCTSTR iconFileName);

/**
* 将HICON保存到ICO格式文件iconFileName中。
*/
extern void SaveIcon(HICON hico, LPCTSTR iconFileName, BOOL bAutoDelete);

/**
* 将HICON保存到BMP格式文件iconFileName中。
*/
extern void SaveIconToBMP(HICON hico, LPCTSTR iconFileName);


#endif  // FILE_SEARCH_WIN_ICON_H_

#ifdef __cplusplus
}
#endif
