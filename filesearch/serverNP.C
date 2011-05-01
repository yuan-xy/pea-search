#include "env.h"
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <process.h>
#include "util.h"
#include "ClntSrvr.h" /* Defines the resquest and response records */

typedef struct { /* Argument to a server thread. */
	HANDLE hNamedPipe; /* Named pipe instance. */
	unsigned int ThreadNo;
} THREAD_ARG;
typedef THREAD_ARG *LPTHREAD_ARG;

volatile static BOOL ShutDown = FALSE;
static unsigned int WINAPI Server (void *);
static unsigned int WINAPI Connect (void *);
static BOOL WINAPI Handler (DWORD);
static WCHAR ShutRqst[] = L"$ShutDownServer";

static HANDLE hSrvrThread[MAX_CLIENTS];
static THREAD_ARG ThArgs[MAX_CLIENTS];

BOOL start_named_pipe(){
	int i;
	for (i = 0; i < MAX_CLIENTS; i++) {
		HANDLE hNp = CreateNamedPipe(SERVER_PIPE, PIPE_ACCESS_DUPLEX,
				PIPE_READMODE_MESSAGE | PIPE_TYPE_MESSAGE | PIPE_WAIT,
				MAX_CLIENTS, 0, 0, INFINITE, NULL);
		if (hNp == INVALID_HANDLE_VALUE) {
			WIN_ERROR;
			return 0;
		}
		ThArgs[i].hNamedPipe = hNp;
		ThArgs[i].ThreadNo = i;
		hSrvrThread[i] = (HANDLE) _beginthreadex(NULL, 0, Server, &ThArgs[i], 0, NULL);
		if (hSrvrThread[i] == NULL) {
			WIN_ERROR;
			return 0;
		}
	}
	return 1;
}

void wait_stop_named_pipe(){
	int i;
	WaitForMultipleObjects(MAX_CLIENTS, hSrvrThread, TRUE, INFINITE);
	for (i = 0; i < MAX_CLIENTS; i++) {
		CloseHandle(hSrvrThread[i]);
	}
	ExitProcess(0);
}

int main(int argc, LPTSTR argv[]) {
	if (!SetConsoleCtrlHandler(Handler, TRUE)) {
		WIN_ERROR;
		return 3;
	}
	if(start_named_pipe()){
		wait_stop_named_pipe();
	}
	return 0;
}

/*  Force the connection thread to shut down if it is still active */
static void exit_conn_thread(HANDLE hConTh){
	DWORD ConThStatus;
	GetExitCodeThread (hConTh, &ConThStatus);
	if (ConThStatus == STILL_ACTIVE) {
		HANDLE hClient = CreateFile (SERVER_PIPE, GENERIC_READ | GENERIC_WRITE, 0, NULL,
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hClient != INVALID_HANDLE_VALUE) CloseHandle (hClient);
		WaitForSingleObject (hConTh, INFINITE);
	}
}

static unsigned int WINAPI Server (void *pArg) {
	LPTHREAD_ARG pThArg = (LPTHREAD_ARG)pArg;
	HANDLE hNamedPipe= pThArg->hNamedPipe, hConTh = NULL;
	while (!ShutDown) {
		DWORD nXfer;
		REQUEST Request;
		RESPONSE Response;
		hConTh = (HANDLE)_beginthreadex (NULL, 0, Connect, hNamedPipe, 0, NULL);
		if (hConTh == NULL) {
			WIN_ERROR;
			_endthreadex(2);
		}
		while (!ShutDown && WaitForSingleObject (hConTh, CS_TIMEOUT) == WAIT_TIMEOUT) {};
		if (ShutDown) continue;
		CloseHandle (hConTh); hConTh = NULL;
		while (!ShutDown && ReadFile (hNamedPipe, &Request, RQ_SIZE, &nXfer, NULL)) {
			Response.Status = 0;
			strcpy ((char *)Response.Record, (char *)Request.Record);
			WriteFile (hNamedPipe, &Response, RS_SIZE, &nXfer, NULL);

			Response.Status = 1; strcpy ((char *)Response.Record, "");
			WriteFile (hNamedPipe, &Response, RS_SIZE, &nXfer, NULL);
		} /* Get next command */
		FlushFileBuffers (hNamedPipe);
		DisconnectNamedPipe (hNamedPipe);
	}
	if(hConTh!=NULL) exit_conn_thread(hConTh);
	printf("Exiting server thread number %d\n", pThArg->ThreadNo);
	_endthreadex (0);
	return 0;
}

static unsigned int WINAPI Connect (void *arg){
	BOOL f= ConnectNamedPipe ((HANDLE)arg, NULL);
	printf ("ConnNP finished: %d\n", f);
	_endthreadex (0);
	return 0;
}

BOOL WINAPI Handler(DWORD CtrlEvent) {
	/* Shutdown the system */
	printf("In console control handler\n");
	ShutDown = TRUE;
	return TRUE;
}
