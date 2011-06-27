#include <stdio.h>
#include "win_icon.h"

int main(){
	HICON hicon = (HICON)LoadImage(GetModuleHandle(TEXT("user32.dll")), 
		MAKEINTRESOURCE(101), IMAGE_ICON, 48, 48, LR_LOADTRANSPARENT);
	HICON hicon2 = LoadIcon(NULL, IDI_WARNING);
	SaveIconToBMP(hicon, TEXT("default.bmp"));
	SaveIcon(hicon, TEXT("default.ico"),TRUE);
	SaveIcon(hicon2, TEXT("default2.ico"),TRUE);
	DestroyIcon(hicon);
	DestroyIcon(hicon2);
	CoInitialize(NULL);
	gen_icon_small(L"c:\\boot.ini", L"small.bmp");
	gen_icon_xlarge(L"E:\\firebreath-1.4\\build\\projects\\PluginGigaso\\Debug\\plugingigaso_wixinstall.pdb",L"ini2.bmp");
	CoUninitialize();
	return 0;
}

