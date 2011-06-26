#include <stdio.h>
#include "win_icon.h"


int main(){
	HICON hico = //::LoadIcon(NULL, IDI_WARNING);
		(HICON)LoadImage(GetModuleHandle(TEXT("user32.dll")), 
		MAKEINTRESOURCE(101), IMAGE_ICON, 48, 48, LR_LOADTRANSPARENT);
	SaveIconToFile(hico, TEXT("default.ico"), TRUE);
	gen_icon_xlarge(L"C:\\boot.ini",L"ini.ico");
	return 0;
}

