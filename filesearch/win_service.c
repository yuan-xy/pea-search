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
#include "common.h"

VOID WINAPI ServiceMain (DWORD argc, LPWSTR argv[]);
VOID WINAPI ServerCtrlHandler(DWORD);
void UpdateStatus (int, int);
int  ServiceSpecific ();

static FILE *hLogFile;
static SERVICE_STATUS hServStatus;
static SERVICE_STATUS_HANDLE hSStat; /* Service status handle for setting status */

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


VOID WINAPI ServiceMain (DWORD argc, LPWSTR argv[]){
	setPWD(NULL);
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

DWORD WINAPI  Hotkey(PVOID pParam){
    MSG msg = {0};
    if (!RegisterHotKey(NULL,1, MOD_ALT, 0x44)) {
        return 1;
    }
    while(GetMessage(&msg, NULL, 0, 0) != 0){
        if (msg.message == WM_HOTKEY){
			wchar_t buffer1[MAX_PATH], buffer2[MAX_PATH];
			get_abs_path(L"search.exe",buffer1);
			get_abs_path(L"",buffer2);
			{
				HINSTANCE  hi = ShellExecute(NULL, L"open", buffer1,NULL,buffer2,SW_SHOW); 
				DWORD ret = GetLastError();
				printf("%d",ret);
			}
			{
        		STARTUPINFO si = {0};
        		PROCESS_INFORMATION pi;
        		si.cb = sizeof(si);
        		if(!CreateProcess(buffer1, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)){
					DWORD ret = GetLastError();
					printf("%d",ret);
        		}
			}

        }
    }
	return 0;
}

int ServiceSpecific (){
	UpdateStatus (-1, -1);
	gigaso_init();
	UpdateStatus (-1, -1);
	if(start_named_pipe()){
		UpdateStatus (SERVICE_RUNNING, -1);
		CreateThread(NULL,0,Hotkey,NULL,0,0);
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
