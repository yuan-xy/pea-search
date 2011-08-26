#include "env.h"
#include <cstdio>  
#include <map>
#include "GIGASOConfig.h"

#include "client/windows/handler/exception_handler.h"
#include "client/windows/sender/crash_report_sender.h"

static google_breakpad::ExceptionHandler *eh; 
static google_breakpad::CrashReportSender *sender;

extern "C" {

#define   ACCESS_READ     1 
#define   ACCESS_WRITE   2 

BOOL   IsAdmin(void)   { 
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
                  printf( "AccessCheck()   failed   with   error   %lu\n ",   GetLastError()); 
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

	static bool SendReport(const wchar_t* dump_path,
                                   const wchar_t* minidump_id,
                                   void* context,
                                   EXCEPTION_POINTERS* exinfo,
                                   MDRawAssertionInfo* assertion,
                                   bool succeeded){
		if(!succeeded) return true;
		wchar_t buffer[MAX_PATH], *p=buffer;
		{
			wcscpy_s(p,MAX_PATH,dump_path);
			p += wcslen(dump_path);
			*p++ = L'\\';
			wcscpy_s(p,MAX_PATH-(p-buffer),minidump_id);
			p += wcslen(minidump_id);
			wcscpy_s(p,MAX_PATH-(p-buffer),L".dmp");
		}
		std::wstring file(buffer),response;
		std::map<std::wstring,std::wstring> map;
		{
			OSVERSIONINFOEX o;
			o.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
			if (GetVersionEx ((LPOSVERSIONINFO) &o)){
				wchar_t osbuf[128];
				wsprintf(osbuf,L"1.%d.%d.%d.%d.%d.%d.%x",o.wProductType,o.dwMajorVersion,o.dwMinorVersion,
					o.wServicePackMajor,o.wServicePackMinor,o.dwBuildNumber,o.wSuiteMask);
				std::wstring os(osbuf);
				map[L"dump[os]"]=os;
				//map.insert(make_pair(L"os",L"os"));
			}
		}
		{
			SYSTEM_INFO info;
			GetSystemInfo(&info); 
			MEMORYSTATUS mem;
			GlobalMemoryStatus(&mem);
			wchar_t cpubuf[128];
			wsprintf(cpubuf,L"%d.%d.%x.%d %d.%d",info.wProcessorArchitecture,info.dwNumberOfProcessors,
				info.wProcessorRevision,info.dwPageSize,mem.dwTotalPhys>>20,mem.dwAvailPhys>>20);
			std::wstring cpu(cpubuf);
			map[L"dump[cpu]"]=cpu;
		}
		{
			wchar_t diskbuf[128];
			DWORD volumeSerialNumber;
			GetVolumeInformation(L"c:\\",NULL,NULL,&volumeSerialNumber,NULL,NULL,NULL,NULL);
			wsprintf(diskbuf,L"%x",volumeSerialNumber);
			std::wstring disk(diskbuf);
			map[L"dump[disk]"]=disk;
		}
		{
			wchar_t verbuf[128];
			wsprintf(verbuf,L"%d.%d",GIGASO_VERSION_MAJOR,GIGASO_VERSION_MINOR);
			std::wstring ver(verbuf);
			map[L"dump[ver]"]=ver;
		}
		{
			wchar_t userbuf[128];
			wchar_t fbuffer[128];
			DWORD size=128;
			GetUserName(fbuffer, &size);
			wsprintf(userbuf,L"%d.%s",IsAdmin(),fbuffer);
			std::wstring user(userbuf);
			map[L"dump[user]"]=user;
		}
		google_breakpad::ReportResult ret = sender->SendCrashReport(L"http://www.1dooo.com:3000/dumps/",map,file,&response);
		return true;
	}
  
	void breakpad_init() {  
		eh = new google_breakpad::ExceptionHandler(L".", NULL, SendReport, NULL,  
			google_breakpad::ExceptionHandler::HANDLER_ALL); 
		sender = new google_breakpad::CrashReportSender(L"gigaso_dump_send");
		//sender->set_max_reports_per_day(5);

	}  

	BOOL request_dump(){
		return eh->WriteMinidump()==true;
	}

}// extern "C"
