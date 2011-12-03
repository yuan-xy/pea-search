#include "env.h"
#include <time.h>
#include <wchar.h>
#include <sys/stat.h>
#include "common.h"
#include "GIGASOConfig.h"

#ifdef WIN32
#include <Shlobj.h>
#else
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/statvfs.h>
#endif

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

BOOL setUserPWD(){
	WCHAR szFilePath[MAX_PATH]={0};	
	SHGetFolderPath(NULL,CSIDL_LOCAL_APPDATA, NULL,SHGFP_TYPE_CURRENT,szFilePath);
	return SetCurrentDirectory(szFilePath);
}

BOOL get_abs_path_exe(const WCHAR *name, WCHAR full_path[]){
	WCHAR *szFileName = NULL;
	GetModuleFileName(NULL,full_path,MAX_PATH);
	szFileName = wcsrchr(full_path,L'\\');
	if(!szFileName) return 0;
	wcscpy(szFileName+1,name);
	return 1;
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
	vswprintf(buffer,255,fmt,args);
	va_end(args);
	OutputDebugString(buffer);
}

BOOL WindowsVersionOK (){
    OSVERSIONINFO v;
	v.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (!GetVersionEx (&v)) return 0;
	if(v.dwPlatformId != VER_PLATFORM_WIN32_NT) return 0;
    return (v.dwMajorVersion==5 && v.dwMinorVersion==1) ||
		(v.dwMajorVersion==6 && v.dwMinorVersion==0) ||
		(v.dwMajorVersion==6 && v.dwMinorVersion==1);
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

BOOL get_os(WCHAR *osbuf){
	OSVERSIONINFOEX o;
	o.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if (GetVersionEx ((LPOSVERSIONINFO) &o)){
		swprintf(osbuf,MAX_PATH,L"1.%d.%d_%d.%d_%d.%d.%x",o.wProductType,o.dwMajorVersion,o.dwMinorVersion,
			o.wServicePackMajor,o.wServicePackMinor,o.dwBuildNumber,o.wSuiteMask);
		return 1;
	}
	return 0;
}

BOOL get_cpu(WCHAR *cpubuf){
	SYSTEM_INFO info, info2;
	MEMORYSTATUS mem;
	GetSystemInfo(&info); 
	GetNativeSystemInfo(&info2); 
	GlobalMemoryStatus(&mem);
	swprintf(cpubuf,MAX_PATH,L"%d_%d.%d.%x.%d %d.%d",info.wProcessorArchitecture,info2.wProcessorArchitecture,info.dwNumberOfProcessors,
		info.wProcessorRevision,info.dwPageSize,mem.dwTotalPhys>>20,mem.dwAvailPhys>>20);
	return 1;
}

BOOL get_disk(WCHAR *diskbuf){
	DWORD volumeSerialNumber;
	GetVolumeInformation(L"c:\\",NULL,0,&volumeSerialNumber,NULL,NULL,NULL,0);
	swprintf(diskbuf,MAX_PATH,L"%x",volumeSerialNumber);
	return 1;
}

BOOL get_user(WCHAR *userbuf){
	WCHAR fbuffer[128];
	DWORD size=128;
	GetUserName(fbuffer, &size);
	swprintf(userbuf,MAX_PATH,L"%d.%s",is_admin(),fbuffer);
	return 1;
}

BOOL get_ver(TCHAR *verbuf){
	swprintf(verbuf,MAX_PATH,L"%d.%d.%d",GIGASO_VERSION_MAJOR,GIGASO_VERSION_MINOR,GIGASO_VERSION_BUILD);
	return 1;
}

#else //WIN32

BOOL get_os(TCHAR *osbuf){
    struct utsname info;
    if(uname(&info)==0){
        snprintf(osbuf,MAX_PATH,"%s %s %s %s",info.sysname,info.nodename, info.release,info.machine);
		return 1;
    }
	return 0;
}

#ifdef APPLE
BOOL get_cpu(TCHAR *cpubuf){
    char model[512];
    uint64_t core;
	uint64_t ncpus;
    uint64_t memsize;
    uint64_t pagesize;
    uint64_t cpuspeed;
    size_t size;
    size = sizeof(model);
    if (sysctlbyname("hw.model", &model, &size, NULL, 0) < 0) {
        return -1;
    }
    size = sizeof(uint64_t);
	sysctlbyname("machdep.cpu.core_count", &core, &size, NULL, 0);
    sysctlbyname("hw.ncpu", &ncpus, &size, NULL, 0);
    sysctlbyname("hw.memsize", &memsize, &size, NULL, 0);
    sysctlbyname("hw.pagesize", &pagesize, &size, NULL, 0);
    sysctlbyname("hw.cpufrequency", &cpuspeed, &size, NULL, 0);
    snprintf(cpubuf,MAX_PATH,"%d %d %d %d %d %s ",core,ncpus, memsize>>20, pagesize, cpuspeed>>20,model);
	return 1;
}
#else
BOOL get_cpu(TCHAR *cpubuf){
	int numcpus = 0;
    char model[255];
    int  cpuspeed;
	char line[512];
	FILE *fpModel = fopen("/proc/cpuinfo", "r");
	if (fpModel) {
		while (fgets(line, 511, fpModel) != NULL) {
			if (strncmp(line, "model name", 10) == 0) {
				numcpus++;
				if (numcpus == 1) {
					char *p = strchr(line, ':') + 2;
					strcpy(model, p);
					model[strlen(model)-1] = 0;
				}
            } else if (strncmp(line, "cpu MHz", 7) == 0) {
                if (numcpus == 1) {
                    sscanf(line, "%*s %*s : %u", &cpuspeed);
                }
            }
        }
        fclose(fpModel);
		snprintf(cpubuf, MAX_PATH, "%d %d %s",numcpus, cpuspeed,model);
        return 0;
    }
    return 1;
}
#endif

BOOL get_disk(TCHAR *diskbuf){
    struct statvfs vbuf;
    if(statvfs("/", &vbuf)==0){
		snprintf(diskbuf,MAX_PATH,"%x",vbuf.f_fsid);
		return 1;
	}
	return 0;
}
BOOL get_user(TCHAR *userbuf){
	snprintf(userbuf,MAX_PATH,"%s",getenv("USER"));
	return 1;
}

BOOL get_ver(TCHAR *verbuf){
	snprintf(verbuf,MAX_PATH,"%d.%d.%d",GIGASO_VERSION_MAJOR,GIGASO_VERSION_MINOR,GIGASO_VERSION_BUILD);
	return 1;
}
#endif

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



#ifdef WIN32
int wchar_to_utf8_len(const WCHAR *in, int insize_c){
	return WideCharToMultiByte(CP_UTF8, 0, (in), (insize_c), NULL, 0, NULL, NULL);
}


INLINE void wchar_to_utf8_nocheck(const WCHAR *in, int insize_c, pUTF8 out, int out_size){
	WideCharToMultiByte(CP_UTF8, 0, in, insize_c, (LPSTR)out, out_size, NULL, NULL);
}

int utf8_to_wchar_len(const pUTF8 in, int insize_b){
	return MultiByteToWideChar(CP_UTF8, 0, in, insize_b, NULL, 0);
}

int utf8_to_wchar_nocheck(const pUTF8 in, int insize_b, WCHAR *out, int out_buffer_size){
	return MultiByteToWideChar(CP_UTF8, 0, in, insize_b, out, out_buffer_size);
}

#else

int wchar_to_utf8_len(const WCHAR *in, int insize_c){
	return wcsnrtombs(NULL, &in, insize_c,0,NULL);
}

void wchar_to_utf8_nocheck(const WCHAR *in, int insize_c, pUTF8 out, int out_size){
	wcsnrtombs(out, &in,insize_c,out_size, NULL);
}

int utf8_to_wchar_len(const pUTF8 in, int insize_b){
	return mbsnrtowcs(NULL, (const char **)&in, insize_b, 0, NULL);
}

int utf8_to_wchar_nocheck(const pUTF8 in, int insize_b, WCHAR *out, int out_buffer_size){
	return mbsnrtowcs(out, (const char **)&in,  insize_b, out_buffer_size, NULL);
}

#endif
