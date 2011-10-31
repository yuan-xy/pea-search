#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_BITMAP_H_
#define FILE_SEARCH_BITMAP_H_

#include "env.h"

#define MAX_HISTORY 20


extern HBITMAP ReplaceColor(HBITMAP hBmp,COLORREF cOldColor,COLORREF cNewColor,HDC hBmpDC);

extern BOOL save_bmp(LPCTSTR bmpFile, HBITMAP hBMP);

extern HRESULT save_as(LPCTSTR pszFile, HBITMAP bmp);

extern HRESULT CreateThumbnail(const WCHAR *wsDir, const WCHAR *wsFile,DWORD dwWidth, DWORD dwHeight, HBITMAP* pThumbnail);


#endif  // FILE_SEARCH_BITMAP_H_

#ifdef __cplusplus
}
#endif
