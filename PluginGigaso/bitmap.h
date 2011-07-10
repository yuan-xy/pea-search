#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_BITMAP_H_
#define FILE_SEARCH_BITMAP_H_

#include "env.h"

#define MAX_HISTORY 20


extern HBITMAP ReplaceColor(HBITMAP hBmp,COLORREF cOldColor,COLORREF cNewColor,HDC hBmpDC);

extern PBITMAPINFO CreateBitmapInfoStruct(HBITMAP hBmp);

extern BOOL CreateBMPFile0(LPCTSTR pszFile, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC);

extern BOOL CreateBMPFile(LPCTSTR pszFile, HBITMAP hBMP);

#endif  // FILE_SEARCH_BITMAP_H_

#ifdef __cplusplus
}
#endif
