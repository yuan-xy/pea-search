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

void print_debug(WCHAR *fmt, ...){
	WCHAR buffer[255];
	va_list args;
	va_start (args, fmt);
	wsprintf(buffer,fmt,args);//TODO:存在问题，只打印了第一个字符
	va_end(args);
	OutputDebugString(buffer);
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

void get_prop(LPCWSTR key, LPWSTR out, DWORD nSize){
	wchar_t buffer[MAX_PATH];
	get_abs_path(L"gigaso.inf",buffer);
	GetPrivateProfileString(L"customize",key,NULL,out,nSize,buffer);
}

void get_prop2(LPCWSTR key, LPWSTR out, DWORD nSize,LPCWSTR deft){
	wchar_t buffer[MAX_PATH];
	get_abs_path(L"gigaso.inf",buffer);
	GetPrivateProfileString(L"customize",key,deft,out,nSize,buffer);
}

void set_prop(LPCWSTR key, LPWSTR str){
	wchar_t buffer[MAX_PATH];
	get_abs_path(L"gigaso.inf",buffer);
	WritePrivateProfileString(L"customize",key,str,buffer);
}

int get_hotkey(){
	wchar_t hotkey[2];
	wchar_t fbuffer[MAX_PATH];
	wchar_t buffer[MAX_PATH];
	DWORD size=MAX_PATH;
	if(GetUserName(fbuffer, &size)){
		wcscat_s(fbuffer,MAX_PATH,L".ini");
		get_abs_path(fbuffer,buffer);
		GetPrivateProfileString(L"customize",L"hotkey",L"0",hotkey,2,buffer);
		return hotkey[0]-L'0';
	}else{
		return 0;
	}
}

BOOL set_hotkey(int key){
	wchar_t hotkey[2];
	wchar_t fbuffer[MAX_PATH];
	wchar_t buffer[MAX_PATH];
	DWORD size=MAX_PATH;
	if(GetUserName(fbuffer, &size)){
		hotkey[0]=key+L'0';
		hotkey[1]=L'\0';
		wcscat_s(fbuffer,MAX_PATH,L".ini");
		get_abs_path(fbuffer,buffer);
		return WritePrivateProfileString(L"customize",L"hotkey",hotkey,buffer);
	}else{
		return 0;
	}

}
