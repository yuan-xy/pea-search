#include "env.h"
#include <locale.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include "fs_common.h"
#include "main.h"
#include "util.h"
#include "serverNP.h"


VOID WINAPI ServiceMain (DWORD argc, LPWSTR argv[]);
VOID WINAPI ServerCtrlHandler(DWORD);
void UpdateStatus (int, int);
int  ServiceSpecific ();

static FILE *hLogFile;
static SERVICE_STATUS hServStatus;
static SERVICE_STATUS_HANDLE hSStat; /* Service status handle for setting status */

#ifdef SERVICE_MODE
VOID main (int argc, LPWSTR argv [])
{
	SERVICE_TABLE_ENTRY DispatchTable[] =
	{
		{ L"",				ServiceMain	},
		{ NULL,						NULL }
	};

	StartServiceCtrlDispatcher (DispatchTable);

	return;
}
#endif

BOOL setPWD(){
	char szFilePath[MAX_PATH]={0};
	char *szFileName = NULL;
	GetModuleFileNameA(NULL,szFilePath,MAX_PATH);
	szFileName = strrchr(szFilePath,'\\');
	if(!szFileName) return 0;
	szFilePath[szFileName-szFilePath]='\0';
	return SetCurrentDirectoryA(szFilePath);
}

VOID WINAPI ServiceMain (DWORD argc, LPWSTR argv[]){
	setPWD();
	hLogFile = fopen ("SimpleServiceLog.txt", "a+");
	if (hLogFile == NULL) return ;
	fprintf(hLogFile, "Starting service. First log entry.");

	hServStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	hServStatus.dwCurrentState = SERVICE_START_PENDING;
	hServStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	hServStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
	hServStatus.dwServiceSpecificExitCode = 0;
	hServStatus.dwCheckPoint = 0;
	hServStatus.dwWaitHint = 3*000; //wait 10 seconds

	hSStat = RegisterServiceCtrlHandler( L"", ServerCtrlHandler);
	if (hSStat == 0) {
		fprintf(hLogFile, "Cannot register control handler");
		hServStatus.dwCurrentState = SERVICE_STOPPED;
		hServStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
		hServStatus.dwServiceSpecificExitCode = 1;
		UpdateStatus (SERVICE_STOPPED, -1);
		return;
	}
	SetServiceStatus (hSStat, &hServStatus);
	/*  Start the service-specific work, now that the generic work is complete */
	if (ServiceSpecific() != 0) {
		hServStatus.dwCurrentState = SERVICE_STOPPED;
		hServStatus.dwServiceSpecificExitCode = 1;  /* Server initilization failed */
		SetServiceStatus (hSStat, &hServStatus);
		return;
	}
	UpdateStatus (SERVICE_STOPPED, 0);
	fclose (hLogFile);
	return;

}

int ServiceSpecific (){
	UpdateStatus (-1, -1);
	gigaso_init();
	UpdateStatus (-1, -1);
	if(start_named_pipe()){
		UpdateStatus (SERVICE_RUNNING, -1);
		wait_stop_named_pipe();
	}
	gigaso_destory();
	hServStatus.dwWin32ExitCode = NO_ERROR;
	hServStatus.dwServiceSpecificExitCode = 0;  
	return 0;
}


VOID WINAPI ServerCtrlHandler( DWORD Control){
	switch (Control) {
	case SERVICE_CONTROL_SHUTDOWN:
	case SERVICE_CONTROL_STOP:
		UpdateStatus (SERVICE_STOP_PENDING, -1);
		shutdown_NP();
		gigaso_destory();
		return;
	case SERVICE_CONTROL_PAUSE:
		break;
	case SERVICE_CONTROL_CONTINUE:
		break;
	case SERVICE_CONTROL_INTERROGATE:
		break;
	default:
		if (Control > 127 && Control < 256) /* User Defined */
		break;
	}
	UpdateStatus (-1, -1);
	return;
}

void UpdateStatus (int NewStatus, int Check)
/*  Set a new service status and checkpoint (either specific value or increment) */
{
	if (Check < 0 ) hServStatus.dwCheckPoint++;
	else			hServStatus.dwCheckPoint = Check;
	if (NewStatus >= 0) hServStatus.dwCurrentState = NewStatus;
	if (!SetServiceStatus (hSStat, &hServStatus)){
			hServStatus.dwCurrentState = SERVICE_STOPPED;
			hServStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
			hServStatus.dwServiceSpecificExitCode = 2;
			UpdateStatus (SERVICE_STOPPED, -1);
			return;
	}
	return;
}
