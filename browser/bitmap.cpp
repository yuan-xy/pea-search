#include <stdio.h>
#include <shtypes.h>
#include <shobjidl.h> 
#include <shlobj.h> 
#include <shlguid.h> 
#include <shellapi.h> 
#include "bitmap.h"
#include <gdiplus.h>
#include <atlimage.h>


#define COLORREF2RGB(Color) (Color & 0xff00) | ((Color >> 16) & 0xff) | ((Color << 16) & 0xff0000)

#ifdef __cplusplus
extern "C" {
#endif


HBITMAP ReplaceColor (HBITMAP hBmp,COLORREF cOldColor,COLORREF cNewColor,HDC hBmpDC)
{
	HBITMAP RetBmp=NULL;
	if (hBmp)
	{	
		HDC BufferDC=CreateCompatibleDC(NULL);	// DC for Source Bitmap
		if (BufferDC)
		{
			HBITMAP hTmpBitmap = (HBITMAP) NULL;
			if (hBmpDC)
				if (hBmp == (HBITMAP)GetCurrentObject(hBmpDC, OBJ_BITMAP))
				{
					hTmpBitmap = CreateBitmap(1, 1, 1, 1, NULL);
					SelectObject(hBmpDC, hTmpBitmap);
				}
			HGDIOBJ PreviousBufferObject=SelectObject(BufferDC,hBmp);
			// here BufferDC contains the bitmap
			
			HDC DirectDC=CreateCompatibleDC(NULL);	// DC for working		
			if (DirectDC)
			{
				// Get bitmap size
				BITMAP bm;
				GetObject(hBmp, sizeof(bm), &bm);
				
				// create a BITMAPINFO with minimal initilisation for the CreateDIBSection
				BITMAPINFO RGB32BitsBITMAPINFO; 
				ZeroMemory(&RGB32BitsBITMAPINFO,sizeof(BITMAPINFO));
				RGB32BitsBITMAPINFO.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
				RGB32BitsBITMAPINFO.bmiHeader.biWidth=bm.bmWidth;
				RGB32BitsBITMAPINFO.bmiHeader.biHeight=bm.bmHeight;
				RGB32BitsBITMAPINFO.bmiHeader.biPlanes=1;
				RGB32BitsBITMAPINFO.bmiHeader.biBitCount=32;
				UINT * ptPixels;	// pointer used for direct Bitmap pixels access

				HBITMAP DirectBitmap= CreateDIBSection(DirectDC, (BITMAPINFO *)&RGB32BitsBITMAPINFO, DIB_RGB_COLORS,(void **)&ptPixels, NULL, 0);
				if (DirectBitmap)
				{
					// here DirectBitmap!=NULL so ptPixels!=NULL no need to test
					HGDIOBJ PreviousObject=SelectObject(DirectDC, DirectBitmap);
					BitBlt(DirectDC,0,0,bm.bmWidth,bm.bmHeight,BufferDC,0,0,SRCCOPY);					
					// here the DirectDC contains the bitmap

					// Convert COLORREF to RGB (Invert RED and BLUE)
					cOldColor=COLORREF2RGB(cOldColor);
					cNewColor=COLORREF2RGB(cNewColor);

					// After all the inits we can do the job : Replace Color
					for (int i=((bm.bmWidth*bm.bmHeight)-1);i>=0;i--)
					{
						if (ptPixels[i]==cOldColor) ptPixels[i]=cNewColor;
					}
					// little clean up
					// Don't delete the result of SelectObject because it's our modified bitmap (DirectBitmap)
					SelectObject(DirectDC,PreviousObject);
					
					// finish
					RetBmp=DirectBitmap;
				}
				// clean up
				DeleteDC(DirectDC);
			}			
			if (hTmpBitmap)
			{
				SelectObject(hBmpDC, hBmp);
				DeleteObject(hTmpBitmap);
			}
			SelectObject(BufferDC,PreviousBufferObject);
			// BufferDC is now useless
			DeleteDC(BufferDC);
		}
	}
	return RetBmp;
}

PBITMAPINFO CreateBitmapInfoStruct(HBITMAP hBmp)
{ 
    BITMAP bmp; 
    PBITMAPINFO pbmi; 
    WORD    cClrBits; 

    // Retrieve the bitmap color format, width, and height.  
    if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp)) return NULL;

    // Convert the color format to a count of bits.  
    cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel); 
    if (cClrBits == 1) 
        cClrBits = 1; 
    else if (cClrBits <= 4) 
        cClrBits = 4; 
    else if (cClrBits <= 8) 
        cClrBits = 8; 
    else if (cClrBits <= 16) 
        cClrBits = 16; 
    else if (cClrBits <= 24) 
        cClrBits = 24; 
    else cClrBits = 32; 

    // Allocate memory for the BITMAPINFO structure. (This structure  
    // contains a BITMAPINFOHEADER structure and an array of RGBQUAD  
    // data structures.)  

     if (cClrBits < 24) 
         pbmi = (PBITMAPINFO) LocalAlloc(LPTR, 
                    sizeof(BITMAPINFOHEADER) + 
                    sizeof(RGBQUAD) * (1<< cClrBits)); 

     // There is no RGBQUAD array for these formats: 24-bit-per-pixel or 32-bit-per-pixel 

     else 
         pbmi = (PBITMAPINFO) LocalAlloc(LPTR, 
                    sizeof(BITMAPINFOHEADER)); 

    // Initialize the fields in the BITMAPINFO structure.  

    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
    pbmi->bmiHeader.biWidth = bmp.bmWidth; 
    pbmi->bmiHeader.biHeight = bmp.bmHeight; 
    pbmi->bmiHeader.biPlanes = bmp.bmPlanes; 
    pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel; 
    if (cClrBits < 24) 
        pbmi->bmiHeader.biClrUsed = (1<<cClrBits); 

    // If the bitmap is not compressed, set the BI_RGB flag.  
    pbmi->bmiHeader.biCompression = BI_RGB; 

    // Compute the number of bytes in the array of color  
    // indices and store the result in biSizeImage.  
    // The width must be DWORD aligned unless the bitmap is RLE 
    // compressed. 
    pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits +31) & ~31) /8
                                  * pbmi->bmiHeader.biHeight; 
    // Set biClrImportant to 0, indicating that all of the  
    // device colors are important.  
     pbmi->bmiHeader.biClrImportant = 0; 
     return pbmi; 
 } 

BOOL CreateBMPFile0(LPCTSTR pszFile, PBITMAPINFO pbi, 
                  HBITMAP hBMP, HDC hDC) 
 { 
     HANDLE hf;                 // file handle  
    BITMAPFILEHEADER hdr;       // bitmap file-header  
    PBITMAPINFOHEADER pbih;     // bitmap info-header  
    LPBYTE lpBits;              // memory pointer  
    DWORD dwTotal;              // total count of bytes  
    DWORD cb;                   // incremental count of bytes  
    BYTE *hp;                   // byte pointer  
    DWORD dwTmp; 

    pbih = (PBITMAPINFOHEADER) pbi; 
    lpBits = (LPBYTE) GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

    if (!lpBits) return 0;

    // Retrieve the color table (RGBQUAD array) and the bits  
    // (array of palette indices) from the DIB.  
    if (!GetDIBits(hDC, hBMP, 0, (WORD) pbih->biHeight, lpBits, pbi, 
        DIB_RGB_COLORS)) 
    {
        return 0;
    }

    // Create the .BMP file.  
    hf = CreateFile(pszFile, 
                   GENERIC_READ | GENERIC_WRITE, 
                   (DWORD) 0, 
                    NULL, 
                   CREATE_ALWAYS, 
                   FILE_ATTRIBUTE_NORMAL, 
                   (HANDLE) NULL); 
    if (hf == INVALID_HANDLE_VALUE) 
        return 0;
    hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M"  
    // Compute the size of the entire file.  
    hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + 
                 pbih->biSize + pbih->biClrUsed 
                 * sizeof(RGBQUAD) + pbih->biSizeImage); 
    hdr.bfReserved1 = 0; 
    hdr.bfReserved2 = 0; 

    // Compute the offset to the array of color indices.  
    hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + 
                    pbih->biSize + pbih->biClrUsed 
                    * sizeof (RGBQUAD); 

    // Copy the BITMAPFILEHEADER into the .BMP file.  
    if (!WriteFile(hf, (LPVOID) &hdr, sizeof(BITMAPFILEHEADER), 
        (LPDWORD) &dwTmp,  NULL)) 
    {
       return 0;
    }

    // Copy the BITMAPINFOHEADER and RGBQUAD array into the file.  
    if (!WriteFile(hf, (LPVOID) pbih, sizeof(BITMAPINFOHEADER) 
                  + pbih->biClrUsed * sizeof (RGBQUAD), 
                  (LPDWORD) &dwTmp, ( NULL)))
        return 0;

    // Copy the array of color indices into the .BMP file.  
    dwTotal = cb = pbih->biSizeImage; 
    hp = lpBits; 
    if (!WriteFile(hf, (LPSTR) hp, (int) cb, (LPDWORD) &dwTmp,NULL)) 
           return 0;

    // Close the .BMP file.  
     if (!CloseHandle(hf)) 
           return 0;

    // Free memory.  
    GlobalFree((HGLOBAL)lpBits);
	return 1;
}


BOOL save_bmp(LPCTSTR pszFile, HBITMAP bmp){
	PBITMAPINFO pbinfo;
	pbinfo = CreateBitmapInfoStruct(bmp);
	return CreateBMPFile0(pszFile,pbinfo,bmp,GetDC(NULL));
}

HRESULT save_as(LPCTSTR pszFile, HBITMAP bmp){
	CImage image;
	image.Attach(bmp);
	return image.Save(pszFile);
}


static wchar_t m_wsBuffer[255];	// 临时缓冲区

HRESULT CreateThumbnail(const wchar_t *wsDir, const wchar_t *wsFile,DWORD dwWidth, DWORD dwHeight, HBITMAP* pThumbnail) {  
	LPITEMIDLIST pidlItems = NULL, pidlURL = NULL, pidlWorkDir = NULL;  
	ULONG   ulParseLen = 0;
	HRESULT hr;  
	WCHAR pszPath[MAX_PATH];  
	DWORD dwPriority = 0, dwFlags = IEIFLAG_ASPECT;  
	SIZE size = { dwWidth, dwHeight };  
	IExtractImage* peiURL = NULL;	// nterface is used to request a thumbnail image from a Shell folder
	IShellFolder* psfDesktop = NULL;  
	IShellFolder* psfWorkDir = NULL;  
	IMalloc*	  pMalloc = NULL;

	// 初始化Com库
	if ( CoInitialize( NULL ) != 0 )
	{
		goto OnExit;
	}

	// 获得IMalloc接口
	hr = SHGetMalloc( &pMalloc );
	if ( FAILED( hr ) )
	{
		goto OnExit;
	}

	// 获得桌面文件夹
	hr = SHGetDesktopFolder(&psfDesktop);  
	if(FAILED(hr)) goto OnExit;  

	// get working directory  
	wcscpy(m_wsBuffer,wsDir);  
	// ParseDisplayName:Translates a file object's or folder's display name into an item identifier list.
	hr = psfDesktop->ParseDisplayName(NULL, NULL, m_wsBuffer, &ulParseLen, &pidlWorkDir, NULL);  
	if(FAILED(hr)) goto OnExit;  
	hr = psfDesktop->BindToObject(pidlWorkDir, NULL, IID_IShellFolder, (LPVOID*)&psfWorkDir);  
	if(FAILED(hr)) goto OnExit;  
	psfDesktop->Release();  
	psfDesktop = NULL;  
	pMalloc->Free(pidlWorkDir);  
	pidlWorkDir = NULL;  

	// retrieve link information  
	wcscpy(m_wsBuffer,wsFile);  
	hr = psfWorkDir->ParseDisplayName(NULL, NULL, m_wsBuffer, &ulParseLen, &pidlURL, NULL);  
	if(FAILED(hr)) goto OnExit;  

	// query IExtractImage  
	hr = psfWorkDir->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST*)&pidlURL, IID_IExtractImage, NULL, (LPVOID*)&peiURL);  
	if(FAILED(hr)) goto OnExit;  

	// define thumbnail properties  
	hr = peiURL->GetLocation(pszPath, MAX_PATH, &dwPriority, &size, 16, &dwFlags);  
	if(FAILED(hr)) goto OnExit;  

	// generate thumbnail  
	hr = peiURL->Extract(pThumbnail);  
	if(FAILED(hr)) goto OnExit;  

	// clean-up IExtractImage  
	peiURL->Release();  
	peiURL = NULL;  

OnExit:  

	// free allocated structures  
	if(peiURL != NULL) peiURL->Release();  
	if(pidlURL != NULL) pMalloc->Free(pidlURL);  
	if(pidlWorkDir != NULL) pMalloc->Free(pidlWorkDir);  
	if(psfDesktop != NULL) psfDesktop->Release();  
	if(psfWorkDir != NULL) psfWorkDir->Release();  
	CoUninitialize();

	return hr;  
}


#ifdef __cplusplus
}
#endif
