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
	DWORD d = GetCurrentDirectoryW(MAX_PATH,full_path);
	if( d==0 ) return 0;
	p += d;
	if(*name != L'\\') *p = L'\\';
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


