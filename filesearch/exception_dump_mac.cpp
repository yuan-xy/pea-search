#include "env.h"
#include "exception_dump.h"

extern "C" {
  
	void breakpad_init() {  
		//TODO
	}  

	BOOL request_dump(){
		return 1;
	}

}// extern "C"
