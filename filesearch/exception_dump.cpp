#include "env.h"
#include <cstdio>  
#include <map>

#include "client/windows/handler/exception_handler.h"
#include "client/windows/sender/crash_report_sender.h"

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
		std::wstring response;
		std::map<std::wstring,std::wstring> map;
		google_breakpad::ReportResult ret = sender->SendCrashReport(L"http://localhost",map,dump_path,&response);
		return true;
	}
  
	void breakpad_init() {  
		eh = new google_breakpad::ExceptionHandler(L".", NULL, SendReport, NULL,  
			google_breakpad::ExceptionHandler::HANDLER_ALL); 
		sender = new google_breakpad::CrashReportSender(L"gigaso_dump_send");
		sender->set_max_reports_per_day(5);

	}  

	BOOL request_dump(){
		return eh->WriteMinidump()==true;
	}

}// extern "C"
