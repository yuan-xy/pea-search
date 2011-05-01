#include "env.h"
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include "util.h"
#include "ClntSrvr.h" /* Defines the resquest and response records */

#include <stdarg.h>	 									  

BOOL PrintStrings (HANDLE hOut, ...)
{
	DWORD MsgLen, Count;
	LPCTSTR pMsg;
	va_list pMsgList;	/* Current message string. */
	va_start (pMsgList, hOut);	/* Start processing msgs. */
	while ((pMsg = va_arg (pMsgList, LPCTSTR)) != NULL) {
		MsgLen = lstrlen (pMsg);
		if (!WriteConsole (hOut, pMsg, MsgLen, &Count, NULL)
				&& !WriteFile (hOut, pMsg, MsgLen * sizeof (TCHAR),
				&Count, NULL))
			return FALSE;
	}
	va_end (pMsgList);
	return TRUE;
}


BOOL PrintMsg (HANDLE hOut, LPCTSTR pMsg){
	return PrintStrings (hOut, pMsg, NULL);
}

BOOL ConsolePrompt (LPCTSTR pPromptMsg, LPTSTR pResponse, DWORD MaxTchar, BOOL Echo){
	HANDLE hStdIn, hStdOut;
	DWORD TcharIn, EchoFlag;
	BOOL Success;
	hStdIn = CreateFile (TEXT ("CONIN$"), GENERIC_READ | GENERIC_WRITE, 0,
			NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	hStdOut = CreateFile (TEXT ("CONOUT$"), GENERIC_WRITE, 0,
			NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	/* Should the input be echoed? */

	EchoFlag = Echo ? ENABLE_ECHO_INPUT : 0;

	/* API "and" chain. If any test or system call fails, the
		rest of the expression is not evaluated, and the
		subsequent functions are not called. GetStdError ()
		will return the result of the failed call. */

	Success = SetConsoleMode (hStdIn, ENABLE_LINE_INPUT | 
			EchoFlag | ENABLE_PROCESSED_INPUT)
			&& SetConsoleMode (hStdOut,
				ENABLE_WRAP_AT_EOL_OUTPUT | ENABLE_PROCESSED_OUTPUT)
			&& PrintStrings (hStdOut, pPromptMsg, NULL)
			&& ReadConsole (hStdIn, pResponse, MaxTchar, &TcharIn, NULL);

	/* Replace the CR-LF by the null character. */

	if (Success)
		pResponse [TcharIn - 2] = '\0';

	CloseHandle (hStdIn);
	CloseHandle (hStdOut);
	return Success;
}


int _tmain (int argc, LPTSTR argv [])
{
	HANDLE hNamedPipe;
	TCHAR PromptMsg [] = _T ("\nEnter Command: ");
	TCHAR QuitMsg [] = _T ("$Quit");
	TCHAR ServerPipeName [MAX_PATH];
	REQUEST Request;		/* See ClntSrvr.h */
	RESPONSE Response;		/* See ClntSrvr.h */
	DWORD nRead, nWrite;

	_tcscpy (ServerPipeName, SERVER_PIPE);
	WaitNamedPipe (ServerPipeName, NMPWAIT_WAIT_FOREVER);
	hNamedPipe = CreateFile (ServerPipeName, GENERIC_READ | GENERIC_WRITE, 0, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hNamedPipe == INVALID_HANDLE_VALUE){
		WIN_ERROR;
		return 3;
	}
	Request.Command = 0;
	Request.RqLen = RQ_SIZE;
	while (ConsolePrompt (PromptMsg, (LPTSTR)Request.Record, MAX_RQRS_LEN, TRUE)
			&& (_tcscmp ((LPTSTR)Request.Record, QuitMsg) != 0)) {

			if (!WriteFile (hNamedPipe, &Request, RQ_SIZE, &nWrite, NULL)){
				WIN_ERROR;
				return 1;
			}

		/* Read each response and send it to std out
			Response.Status == 0 indicates "end of response". */

		while (ReadFile (hNamedPipe, &Response, RS_SIZE, &nRead, NULL)
				&& (Response.Status == 0))
			_tprintf (_T ("%s"), Response.Record);

	}
	CloseHandle (hNamedPipe);
	return 0;
}

