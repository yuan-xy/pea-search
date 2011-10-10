#include "env.h"
#include <time.h>
#include <sys/stat.h>
#include "common.h"
#include "GIGASOConfig.h"

#ifdef WIN32
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

#define DEFAULT_HOT_KEY 6
int get_hotkey(){
	wchar_t fbuffer[MAX_PATH];
	wchar_t buffer[MAX_PATH];
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

#define   ACCESS_READ     1 
#define   ACCESS_WRITE   2 

BOOL is_admin(void){ 
      HANDLE   hToken; 
      DWORD     dwStatus; 
      DWORD     dwAccessMask; 
      DWORD     dwAccessDesired; 
      DWORD     dwACLSize; 
      DWORD     dwStructureSize   =   sizeof(PRIVILEGE_SET); 
      PACL       pACL                         =   NULL; 
      PSID       psidAdmin               =   NULL; 
      BOOL       bReturn                   =   FALSE; 

      PRIVILEGE_SET       ps; 
      GENERIC_MAPPING   GenericMapping; 

      PSECURITY_DESCRIPTOR           psdAdmin                       =   NULL; 
      SID_IDENTIFIER_AUTHORITY   SystemSidAuthority   =   SECURITY_NT_AUTHORITY; 
      
      __try   { 

            //   AccessCheck()   requires   an   impersonation   token. 
            ImpersonateSelf(SecurityImpersonation); 

            if   (!OpenThreadToken(GetCurrentThread(),   TOKEN_QUERY,   FALSE,   
                        &hToken))   { 

                  if   (GetLastError()   !=   ERROR_NO_TOKEN) 
                        __leave; 

                  //   If   the   thread   does   not   have   an   access   token,   we 'll   
                  //   examine   the   access   token   associated   with   the   process. 
                  if   (!OpenProcessToken(GetCurrentProcess(),   TOKEN_QUERY,   
                              &hToken)) 
                        __leave; 
            } 

            if   (!AllocateAndInitializeSid(&SystemSidAuthority,   2,   
                        SECURITY_BUILTIN_DOMAIN_RID,   DOMAIN_ALIAS_RID_ADMINS, 
                        0,   0,   0,   0,   0,   0,   &psidAdmin)) 
                  __leave; 

            psdAdmin   =   LocalAlloc(LPTR,   SECURITY_DESCRIPTOR_MIN_LENGTH); 
            if   (psdAdmin   ==   NULL) 
                  __leave; 

            if   (!InitializeSecurityDescriptor(psdAdmin, 
                        SECURITY_DESCRIPTOR_REVISION)) 
                  __leave; 
    
            //   Compute   size   needed   for   the   ACL. 
            dwACLSize   =   sizeof(ACL)   +   sizeof(ACCESS_ALLOWED_ACE)   + 
                        GetLengthSid(psidAdmin)   -   sizeof(DWORD); 

            //   Allocate   memory   for   ACL. 
            pACL   =   (PACL)LocalAlloc(LPTR,   dwACLSize); 
            if   (pACL   ==   NULL) 
                  __leave; 

            //   Initialize   the   new   ACL. 
            if   (!InitializeAcl(pACL,   dwACLSize,   ACL_REVISION2)) 
                  __leave; 

            dwAccessMask=   ACCESS_READ   |   ACCESS_WRITE; 
            
            //   Add   the   access-allowed   ACE   to   the   DACL. 
            if   (!AddAccessAllowedAce(pACL,   ACL_REVISION2, 
                        dwAccessMask,   psidAdmin)) 
                  __leave; 

            //   Set   our   DACL   to   the   SD. 
            if   (!SetSecurityDescriptorDacl(psdAdmin,   TRUE,   pACL,   FALSE)) 
                  __leave; 

            //   AccessCheck   is   sensitive   about   what   is   in   the   SD;   set 
            //   the   group   and   owner. 
            SetSecurityDescriptorGroup(psdAdmin,   psidAdmin,   FALSE); 
            SetSecurityDescriptorOwner(psdAdmin,   psidAdmin,   FALSE); 

            if   (!IsValidSecurityDescriptor(psdAdmin)) 
                  __leave; 

            dwAccessDesired   =   ACCESS_READ; 

            //   
            //   Initialize   GenericMapping   structure   even   though   we 
            //   won 't   be   using   generic   rights. 
            //   
            GenericMapping.GenericRead         =   ACCESS_READ; 
            GenericMapping.GenericWrite       =   ACCESS_WRITE; 
            GenericMapping.GenericExecute   =   0; 
            GenericMapping.GenericAll           =   ACCESS_READ   |   ACCESS_WRITE; 

            if   (!AccessCheck(psdAdmin,   hToken,   dwAccessDesired,   
                        &GenericMapping,   &ps,   &dwStructureSize,   &dwStatus,   
                        &bReturn))   { 
                  //printf( "AccessCheck()   failed   with   error   %lu\n ",   GetLastError()); 
                  __leave; 
            } 

            RevertToSelf(); 
      
      }   __finally   { 

            //   Cleanup   
            if   (pACL)   LocalFree(pACL); 
            if   (psdAdmin)   LocalFree(psdAdmin);     
            if   (psidAdmin)   FreeSid(psidAdmin); 
      } 

      return   bReturn; 
} 

BOOL get_os(wchar_t *osbuf){
	OSVERSIONINFOEX o;
	o.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if (GetVersionEx ((LPOSVERSIONINFO) &o)){
		wsprintf(osbuf,L"1.%d.%d_%d.%d_%d.%d.%x",o.wProductType,o.dwMajorVersion,o.dwMinorVersion,
			o.wServicePackMajor,o.wServicePackMinor,o.dwBuildNumber,o.wSuiteMask);
		return 1;
	}
	return 0;
}

BOOL get_cpu(wchar_t *cpubuf){
	SYSTEM_INFO info, info2;
	MEMORYSTATUS mem;
	GetSystemInfo(&info); 
	GetNativeSystemInfo(&info2); 
	GlobalMemoryStatus(&mem);
	wsprintf(cpubuf,L"%d_%d.%d.%x.%d %d.%d",info.wProcessorArchitecture,info2.wProcessorArchitecture,info.dwNumberOfProcessors,
		info.wProcessorRevision,info.dwPageSize,mem.dwTotalPhys>>20,mem.dwAvailPhys>>20);
	return 1;
}

BOOL get_disk(wchar_t *diskbuf){
	DWORD volumeSerialNumber;
	GetVolumeInformation(L"c:\\",NULL,0,&volumeSerialNumber,NULL,NULL,NULL,0);
	wsprintf(diskbuf,L"%x",volumeSerialNumber);
	return 1;
}

BOOL get_user(wchar_t *userbuf){
	wchar_t fbuffer[128];
	DWORD size=128;
	GetUserName(fbuffer, &size);
	wsprintf(userbuf,L"%d.%s",is_admin(),fbuffer);
	return 1;
}
#endif

BOOL get_ver(wchar_t *verbuf){
	swprintf(verbuf,16,L"%d.%d.%d",GIGASO_VERSION_MAJOR,GIGASO_VERSION_MINOR,GIGASO_VERSION_BUILD);
	return 1;
}

BOOL passed_one_day(time_t last){
	time_t now = time(NULL);
	return now-last>3600*24;
}

BOOL file_passed_one_day(char *filename){
	struct stat statbuf;
	int ret=stat(filename, &statbuf);
	if(ret==-1) return 1;
	return passed_one_day(statbuf.st_mtime);
}