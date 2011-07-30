#include <atlbase.h>
#include <shlobj.h> 
#include <shlguid.h> 
#include <shellapi.h> 
#include <commctrl.h> 
#include <commoncontrols.h> 
#include <stdio.h>
#include "win_icon.h"
#include "bitmap.h"


#ifdef __cplusplus
extern "C" {
#endif

void gen_icon_xlarge(LPCTSTR szFileName, LPCTSTR iconFileName){
	SHFILEINFOW sfi = {0}; 
	SHGetFileInfo(szFileName, -1, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_ADDOVERLAYS | SHGFI_ICON ); 
	HIMAGELIST* imageList; 
	HRESULT hResult = SHGetImageList(SHIL_EXTRALARGE, IID_IImageList, (void**)&imageList);  
	if (hResult == S_OK) { 
	  // Get the icon we need from the list. Note that the HIMAGELIST we retrieved 
	  // earlier needs to be casted to the IImageList interface before use. 
	  HICON hIcon; 
	  hResult = ((IImageList*)imageList)->GetIcon(sfi.iIcon, ILD_TRANSPARENT, &hIcon); 
	  if (hResult == S_OK) { 
		 save_icon_to_photo(hIcon, iconFileName);
		 //SaveIcon(hIcon,iconFileName,TRUE);
		 DestroyIcon(hIcon);
	  } 
	} 
}

BOOL gen_icon_small(LPCTSTR szFileName, LPCTSTR iconFileName){
	SHFILEINFO shfi;
	HRESULT hr = SHGetFileInfo( szFileName, 0, &shfi, sizeof( SHFILEINFO ), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS );
	if( !SUCCEEDED(hr) || shfi.hIcon==NULL) return 0;
	save_icon_to_photo(shfi.hIcon, iconFileName);
	//SaveIcon(shfi.hIcon,iconFileName,TRUE);
	DestroyIcon(shfi.hIcon);
	return 1;
}

void saveicon(HICON hico, LPCTSTR szFileName, BOOL bAutoDelete){
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

void save_icon_to_photo(HICON hico, LPCTSTR szFileName){	
	HDC hdc = GetDC(NULL) ;
	ICONINFO info;
	GetIconInfo(hico,&info);
	HBITMAP  bmp = ReplaceColor(info.hbmColor,RGB(0,0,0), RGB(255,255,255), hdc);
	save_as(szFileName,bmp);
}


#ifdef __cplusplus
}
#endif
