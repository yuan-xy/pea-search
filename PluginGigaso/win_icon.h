#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_WIN_ICON_H_
#define FILE_SEARCH_WIN_ICON_H_

#include "env.h"

#define MAX_HISTORY 20

extern void gen_icon_xlarge(LPCTSTR szFileName, LPCTSTR iconFileName);

extern void gen_icon_small(LPCTSTR szFileName, LPCTSTR iconFileName);

extern void SaveIconToFile(HICON hico, LPCTSTR szFileName, BOOL bAutoDelete);


#endif  // FILE_SEARCH_WIN_ICON_H_

#ifdef __cplusplus
}
#endif
