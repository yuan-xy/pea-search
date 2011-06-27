#include <atlbase.h>
#include <shlobj.h> 
#include <shlguid.h> 
#include <shellapi.h> 
#include <commctrl.h> 
#include <commoncontrols.h> 
#include <stdio.h>
#include "win_icon.h"

#define COLORREF2RGB(Color) (Color & 0xff00) | ((Color >> 16) & 0xff) | ((Color << 16) & 0xff0000)

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

#ifdef __cplusplus
extern "C" {
#endif

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

BOOL CreateBMPFile(LPCTSTR pszFile, PBITMAPINFO pbi, 
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

void gen_icon_xlarge(LPCTSTR szFileName, LPCTSTR iconFileName){
	SHFILEINFOW sfi = {0}; 
	SHGetFileInfo(szFileName, -1, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX); 
	HIMAGELIST* imageList; 
	HRESULT hResult = SHGetImageList(SHIL_EXTRALARGE, IID_IImageList, (void**)&imageList);  
	if (hResult == S_OK) { 
	  // Get the icon we need from the list. Note that the HIMAGELIST we retrieved 
	  // earlier needs to be casted to the IImageList interface before use. 
	  HICON hIcon; 
	  hResult = ((IImageList*)imageList)->GetIcon(sfi.iIcon, ILD_TRANSPARENT, &hIcon); 
	  if (hResult == S_OK) { 
		 SaveIconToBMP(hIcon, iconFileName);
		 //SaveIcon(hIcon,iconFileName,TRUE);
		 DestroyIcon(hIcon);
	  } 
	} 
}

void gen_icon_small(LPCTSTR szFileName, LPCTSTR iconFileName){
	SHFILEINFO shfi;
	HRESULT hr = SHGetFileInfo( szFileName, 0, &shfi, sizeof( SHFILEINFO ), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS );
	SaveIconToBMP(shfi.hIcon, iconFileName);
	//SaveIcon(shfi.hIcon,iconFileName,TRUE);
	DestroyIcon(shfi.hIcon);
}

void SaveIcon(HICON hico, LPCTSTR szFileName, BOOL bAutoDelete){
	PICTDESC pd = {sizeof(pd), PICTYPE_ICON};
	pd.icon.hicon = hico;

	CComPtr<IPicture> pPict = NULL;
	CComPtr<IStream>  pStrm = NULL;
	LONG cbSize = 0;

	BOOL res = FALSE;

	res = SUCCEEDED( ::CreateStreamOnHGlobal(NULL, TRUE, &pStrm) );
	res = SUCCEEDED( ::OleCreatePictureIndirect(&pd, IID_IPicture, bAutoDelete, (void**)&pPict) );
	res = SUCCEEDED( pPict->SaveAsFile( pStrm, TRUE, &cbSize ) );

	if( res )
	{
		// rewind stream to the beginning
		LARGE_INTEGER li = {0};
		pStrm->Seek(li, STREAM_SEEK_SET, NULL);

		// write to file
		HANDLE hFile = ::CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
		if( INVALID_HANDLE_VALUE != hFile )
		{
			DWORD dwWritten = 0, dwRead = 0, dwDone = 0;
			BYTE  buf[4096];
			while( dwDone < (DWORD)cbSize )
			{
				if( SUCCEEDED(pStrm->Read(buf, sizeof(buf), &dwRead)) )
				{
					::WriteFile(hFile, buf, dwRead, &dwWritten, NULL);
					if( dwWritten != dwRead )
						break;
					dwDone += dwRead;
				}
				else
					break;
			}

			_ASSERTE(dwDone == cbSize);
			::CloseHandle(hFile);
		}
	}
}

void SaveIconToBMP(HICON hico, LPCTSTR szFileName){	
	HDC hdc = GetDC(NULL) ;
	ICONINFO info;
	GetIconInfo(hico,&info);
	HBITMAP  bmp = ReplaceColor(info.hbmColor,RGB(0,0,0), RGB(255,255,255), hdc);
	PBITMAPINFO pbinfo;
	pbinfo = CreateBitmapInfoStruct(bmp);
	CreateBMPFile(szFileName,pbinfo,bmp,hdc);
}


#ifdef __cplusplus
}
#endif
