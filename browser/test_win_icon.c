#include <stdio.h>
#include "win_icon.h"
#include "bitmap.h"

void gen_thumbnail(const wchar_t *wsDir, const wchar_t *wsFile, const wchar_t *thumb_name){
	HBITMAP pThumbnail;
	HRESULT hr = CreateThumbnail(wsDir,wsFile,100,100,&pThumbnail);
	if( SUCCEEDED( hr ) ){
			save_bmp(thumb_name, pThumbnail);
	}
}

int main(){
	HICON hicon = (HICON)LoadImage(GetModuleHandle(TEXT("user32.dll")), 
		MAKEINTRESOURCE(101), IMAGE_ICON, 48, 48, LR_LOADTRANSPARENT);
	HICON hicon2 = LoadIcon(NULL, IDI_WARNING);
	save_icon_to_photo(hicon, TEXT("default.jpg"));
	save_icon_to_photo(hicon, TEXT("default.bmp"));
	saveicon(hicon, TEXT("default.ico"),TRUE);
	saveicon(hicon2, TEXT("default2.ico"),TRUE);
	DestroyIcon(hicon);
	DestroyIcon(hicon2);
	CoInitialize(NULL);
	gen_icon_small(L"c:\\boot.ini", L"small.bmp");
	gen_icon_xlarge(L"E:\\firebreath-1.4\\build\\projects\\PluginGigaso\\Debug\\plugingigaso_wixinstall.pdb",L"ini2.bmp");
	gen_icon_xlarge(L"E:\\firebreath-1.4\\build\\projects\\PluginGigaso\\Debug\\plugingigaso_wixinstall.pdb",L"ini2.gif");
	CoUninitialize();
	gen_thumbnail(L"C:\\google-d", L"1.xps", L"setup_thumb.bmp");
	return 0;
}

