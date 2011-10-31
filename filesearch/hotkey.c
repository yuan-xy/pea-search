#include "env.h"
#include <time.h>
#include <wchar.h>
#include <sys/stat.h>
#include "common.h"
#include "GIGASOConfig.h"

#ifdef WIN32
#define DEFAULT_HOT_KEY 6
int get_hotkey(){
	WCHAR fbuffer[MAX_PATH];
	WCHAR buffer[MAX_PATH];
	DWORD size=MAX_PATH;
	if(GetUserName(fbuffer, &size)){
		wcscat_s(fbuffer,MAX_PATH,L".ini");
		get_abs_path(fbuffer,buffer);
		return GetPrivateProfileInt(L"customize",L"hotkey",DEFAULT_HOT_KEY,buffer);
	}else{
		return DEFAULT_HOT_KEY;//VK_F9
	}
}
#undef DEFAULT_HOT_KEY


BOOL set_hotkey(int key){
	WCHAR hotkey[2];
	WCHAR fbuffer[MAX_PATH];
	WCHAR buffer[MAX_PATH];
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
#else //WIN32



#endif
