#include "env.h"
#include "util.h"
#include "server.h"
#include "main.h"
#include "drive.h"
#include "util.h"


int main(){
	if (!SetConsoleCtrlHandler(shutdown_handle, TRUE)) {
		WIN_ERROR;
		return 3;
	}
	//get_drive_space(10);
	gigaso_init();
	if(start_named_pipe()){
		wait_stop_named_pipe();
	}
	gigaso_destory();
	ExitProcess(0);
	return 0;
}
