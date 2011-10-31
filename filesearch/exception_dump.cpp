#include "env.h"
#include "common.h"
#include <cstdio>  
#include <map>

#include "client/windows/handler/exception_handler.h"
#include "client/windows/sender/crash_report_sender.h"

static google_breakpad::ExceptionHandler *eh; 
static google_breakpad::CrashReportSender *sender;

extern "C" {

	static bool SendReport(const WCHAR* dump_path,
                                   const WCHAR* minidump_id,
                                   void* context,
                                   EXCEPTION_POINTERS* exinfo,
                                   MDRawAssertionInfo* assertion,
                                   bool succeeded){
		if(!succeeded) return true;
		WCHAR buffer[MAX_PATH], *p=buffer;
		{
			wcscpy_s(p,MAX_PATH,dump_path);
			p += wcslen(dump_path);
			#ifdef WIN32
				*p++ = L'\\';
			#else
			    *p++ = L'/';
			#endif
			wcscpy_s(p,MAX_PATH-(p-buffer),minidump_id);
			p += wcslen(minidump_id);
			wcscpy_s(p,MAX_PATH-(p-buffer),L".dmp");
		}
		std::wstring file(buffer),response;
		std::map<std::wstring,std::wstring> map;
		{
			WCHAR osbuf[MAX_PATH];
			get_os(osbuf);
			std::wstring os(osbuf);
			map[L"dump[os]"]=os;
			//map.insert(make_pair(L"os",L"os"));
		}
		{
			WCHAR cpubuf[MAX_PATH];
			get_cpu(cpubuf);
			std::wstring cpu(cpubuf);
			map[L"dump[cpu]"]=cpu;
		}
		{
			WCHAR diskbuf[MAX_PATH];
			get_disk(diskbuf);
			std::wstring disk(diskbuf);
			map[L"dump[disk]"]=disk;
		}
		{
			WCHAR verbuf[MAX_PATH];
			get_ver(verbuf);
			std::wstring ver(verbuf);
			map[L"dump[ver]"]=ver;
		}
		{
			WCHAR userbuf[MAX_PATH];
			get_user(userbuf);
			std::wstring user(userbuf);
			map[L"dump[user]"]=user;
		}
		google_breakpad::ReportResult ret = sender->SendCrashReport(L"http://www.wandouss.com/dumps/",map,file,&response);
		if(ret==google_breakpad::RESULT_FAILED) sender->SendCrashReport(L"http://60.191.119.190:3333/dumps/",map,file,&response);
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
