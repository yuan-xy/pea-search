#include "env.h"
#include "common.h"
#include <cstdio>  
#include <map>

#ifdef WIN32
#include "client/windows/handler/exception_handler.h"
#include "client/windows/sender/crash_report_sender.h"
#else
#include "client/mac/handler/exception_handler.h"
#include "client/mac/sender/crash_report_sender.h"
#endif

static google_breakpad::ExceptionHandler *eh; 
static google_breakpad::CrashReportSender *sender;

extern "C" {

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
			wchar_t osbuf[128];
			get_os(osbuf);
			std::wstring os(osbuf);
			map[L"dump[os]"]=os;
			//map.insert(make_pair(L"os",L"os"));
		}
		{
			wchar_t cpubuf[128];
			get_cpu(cpubuf);
			std::wstring cpu(cpubuf);
			map[L"dump[cpu]"]=cpu;
		}
		{
			wchar_t diskbuf[128];
			get_disk(diskbuf);
			std::wstring disk(diskbuf);
			map[L"dump[disk]"]=disk;
		}
		{
			wchar_t verbuf[128];
			get_ver(verbuf);
			std::wstring ver(verbuf);
			map[L"dump[ver]"]=ver;
		}
		{
			wchar_t userbuf[128];
			get_user(userbuf);
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
		sender->set_max_reports_per_day(3);

	}  

	BOOL request_dump(){
		return eh->WriteMinidump()==true;
	}

}// extern "C"
