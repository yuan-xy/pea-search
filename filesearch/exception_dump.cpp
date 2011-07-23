#include "env.h"
#include <cstdio>  

#ifndef _DEBUG
	#include "client/windows/handler/exception_handler.h"
	static google_breakpad::ExceptionHandler *eh; 
#endif

extern "C" {
  
	void breakpad_init() {  
#ifndef _DEBUG
		eh = new google_breakpad::ExceptionHandler(L".", NULL, NULL, NULL,  
			google_breakpad::ExceptionHandler::HANDLER_ALL); 
#endif

	}  

}// extern "C"
