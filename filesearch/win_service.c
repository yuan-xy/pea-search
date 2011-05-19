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


void LogEvent (LPCWSTR, DWORD, BOOL);
VOID WINAPI ServiceMain (DWORD argc, LPWSTR argv[]);
VOID WINAPI ServerCtrlHandler(DWORD);
void UpdateStatus (int, int);
int  ServiceSpecific (int, LPWSTR *);

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

VOID WINAPI ServiceMain (DWORD argc, LPWSTR argv[]){
	DWORD i;
	if (argc > 2) SetCurrentDirectory (argv[2]);
	hLogFile = fopen ("SimpleServiceLog.txt", "w+");
	if (hLogFile == NULL) return ;
	LogEvent(L"Starting service. First log entry.", 0, FALSE);
	fprintf (hLogFile, "\nargc = %d", argc);
	for (i = 0; i < argc; i++) 
		fprintf (hLogFile, "\nargv[%d] = %ls", i, argv[i]);
	LogEvent(L"Entering ServiceMain.", 0, FALSE);

	hServStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	hServStatus.dwCurrentState = SERVICE_START_PENDING;
	hServStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	hServStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
	hServStatus.dwServiceSpecificExitCode = 0;
	hServStatus.dwCheckPoint = 0;
	hServStatus.dwWaitHint = 3*000; //wait 10 seconds

	hSStat = RegisterServiceCtrlHandler( L"", ServerCtrlHandler);
	if (hSStat == 0) 
		LogEvent (L"Cannot register control handler", 100, TRUE);
	SetServiceStatus (hSStat, &hServStatus);
	/*  Start the service-specific work, now that the generic work is complete */
	if (ServiceSpecific (argc, argv) != 0) {
		hServStatus.dwCurrentState = SERVICE_STOPPED;
		hServStatus.dwServiceSpecificExitCode = 1;  /* Server initilization failed */
		SetServiceStatus (hSStat, &hServStatus);
		return;
	}
	LogEvent (L"Service threads shut down. Set SERVICE_STOPPED status", 0, FALSE);
	UpdateStatus (SERVICE_STOPPED, 0);
	fclose (hLogFile);
	return;

}

int ServiceSpecific (int argc, LPWSTR argv[]){
	UpdateStatus (-1, -1); /* Now change to status; increment the checkpoint */
	setlocale (LC_ALL, "");
	my_assert(24 == sizeof(FileEntry), 1);
	gigaso_init();
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
		LogEvent(L"Shutdown...", 0, FALSE);
		shutdown_NP();
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
	if (!SetServiceStatus (hSStat, &hServStatus))
		LogEvent (L"Cannot set service status", 101, TRUE);
	return;
}


VOID LogEvent (LPCWSTR UserMessage, DWORD ExitCode, BOOL PrintErrorMsg)

/*  General-purpose function for reporting system errors.
	Obtain the error number and turn it into the system error message.
	Display this information and the user-specified message to the open log FILE
	UserMessage:		Message to be displayed to standard error device.
	ExitCode:			0 - Return.
						> 0 - ExitProcess with this code.
	PrintErrorMessage:	Display the last system error message if this flag is set. */
{
	DWORD eMsgLen, ErrNum = GetLastError ();
	LPWSTR lpvSysMsg;
	TCHAR MessageBuffer[512];

//	ALTERNATIVE: Use a registry event log, as shown, or
//		OutputDebugString.
//	HANDLE hEventSource;

//	hEventSource = RegisterEventSource (NULL, ServiceName);
	/*  Not much to do if this fails but to keep trying. */

//	if (hEventSource != NULL) {
//		ReportEvent (hEventSource, EVENTLOG_WARNING_TYPE,
//			0,0, NULL, 1, 0, &UserMessage, NULL);
//	}

	if (PrintErrorMsg) {
		eMsgLen = FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM, NULL,
			ErrNum, MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPWSTR)&lpvSysMsg, 0, NULL);

		_swprintf (MessageBuffer, L"\n%s %s ErrNum = %d. ExitCode = %d.",
			UserMessage, lpvSysMsg, ErrNum, ExitCode);
		HeapFree (GetProcessHeap (), 0, lpvSysMsg);
				/* Explained in Chapter 6. */
	} else {
		_swprintf (MessageBuffer, L"\n%s ExitCode = %d.",
			UserMessage, ExitCode);
	}

	fputs (MessageBuffer, hLogFile);

//		ReportEvent (hEventSource, 
//			ExitCode > 0 ? EVENTLOG_ERROR_TYPE : EVENTLOG_WARNING_TYPE,
//			0, 0, NULL, 1, 0, (LPCWSTR*)&lpvSysMsg, NULL); 

	
	if (ExitCode > 0)
		ExitProcess (ExitCode);
	else
		return;
}


