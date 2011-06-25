#include "env.h"
#include "util.h"
#include "serverNP.h"
#include "main.h"

int main(){
	if (!SetConsoleCtrlHandler(shutdown_handle, TRUE)) {
		WIN_ERROR;
		return 3;
	}
	gigaso_init();
	if(start_named_pipe()){
		wait_stop_named_pipe();
	}
	gigaso_destory();
	ExitProcess(0);
	return 0;
}
