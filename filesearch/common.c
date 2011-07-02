#include <windows.h>
#include "common.h"

BOOL setPWD(char *lpModuleName){
	char szFilePath[MAX_PATH]={0};
	char *szFileName = NULL;
	HMODULE hModule = NULL;
	if(lpModuleName!=NULL) hModule = GetModuleHandleA(lpModuleName);
	GetModuleFileNameA(hModule,szFilePath,MAX_PATH);
	szFileName = strrchr(szFilePath,'\\');
	if(!szFileName) return 0;
	szFilePath[szFileName-szFilePath]='\0';
	return SetCurrentDirectoryA(szFilePath);
}

BOOL get_abs_path(const WCHAR *name, WCHAR full_path[]){
	WCHAR *p = full_path;
	if( !GetCurrentDirectoryW(MAX_PATH,full_path) ) return 0;
	p += wcslen(full_path);
	*p++ = L'\\';
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


