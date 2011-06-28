#include <windows.h>
#include "common.h"

BOOL setPWD(){
	char szFilePath[MAX_PATH]={0};
	char *szFileName = NULL;
	GetModuleFileNameA(NULL,szFilePath,MAX_PATH);
	szFileName = strrchr(szFilePath,'\\');
	if(!szFileName) return 0;
	szFilePath[szFileName-szFilePath]='\0';
	return SetCurrentDirectoryA(szFilePath);
}

BOOL get_abs_path(const WCHAR *name, WCHAR full_path[]){
	WCHAR *p;
	if( !GetModuleFileNameW( NULL, full_path, MAX_PATH ) ) return 0;
	p = wcsrchr(full_path,'\\');
	if(p==NULL) return 0;
	*(p+1) = L'\0';
	#pragma warning(suppress:4996)
	wcscat(p,name);
	return 1;
}

BOOL WindowsVersionOK (DWORD MajorVerRequired, DWORD MinorVerRequired){
    OSVERSIONINFO OurVersion;
	OurVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (!GetVersionEx (&OurVersion)) return 0;
    return ( (OurVersion.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
		       ((OurVersion.dwMajorVersion > MajorVerRequired) ||
                (OurVersion.dwMajorVersion >= MajorVerRequired &&
			     OurVersion.dwMinorVersion >= MinorVerRequired) ));
}


