#include "env.h"
#include <aclapi.h>
#include <stdio.h>
#include "sharelib.h"
#include "server.h"
#include "serverNP.h"
#include "util.h"

volatile static BOOL ShutDown = FALSE;
static HANDLE hSrvrThread = NULL; 

static PSID pEveryoneSID=NULL, pAdminSID=NULL;
static PACL pACL=NULL;
static PSECURITY_DESCRIPTOR pSD=NULL;
static EXPLICIT_ACCESS ea[2];
static SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
static SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
static SECURITY_ATTRIBUTES sa;

static BOOL init_right(){
    if(!AllocateAndInitializeSid(&SIDAuthWorld, 1,
                     SECURITY_WORLD_RID,
                     0, 0, 0, 0, 0, 0, 0,
                     &pEveryoneSID))
    {
        return 0; 
    }
 
    ZeroMemory(&ea, 2 * sizeof(EXPLICIT_ACCESS));
    ea[0].grfAccessPermissions = GENERIC_READ | GENERIC_WRITE;
    ea[0].grfAccessMode = SET_ACCESS;
    ea[0].grfInheritance= NO_INHERITANCE;
    ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea[0].Trustee.ptstrName  = (LPTSTR) pEveryoneSID;

    if(! AllocateAndInitializeSid(&SIDAuthNT, 2,
                     SECURITY_BUILTIN_DOMAIN_RID,
                     DOMAIN_ALIAS_RID_ADMINS,
                     0, 0, 0, 0, 0, 0,
                     &pAdminSID)) 
    {
        return 0; 
    }
 
    ea[1].grfAccessPermissions = GENERIC_ALL | KEY_ALL_ACCESS;
    ea[1].grfAccessMode = SET_ACCESS;
    ea[1].grfInheritance= NO_INHERITANCE;
    ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ea[1].Trustee.ptstrName  = (LPTSTR) pAdminSID;

    if (ERROR_SUCCESS != SetEntriesInAcl(2, ea, NULL, &pACL)) 
    {
        return 0; 
    }
 
    // Initialize a security descriptor.  
    pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, 
                             SECURITY_DESCRIPTOR_MIN_LENGTH); 
    if (NULL == pSD) 
    { 
        return 0; 
    } 
 
    if (!InitializeSecurityDescriptor(pSD,
            SECURITY_DESCRIPTOR_REVISION)) 
    {  
        return 0; 
    } 
 
    // Add the ACL to the security descriptor. 
    if (!SetSecurityDescriptorDacl(pSD, 
            TRUE,     // bDaclPresent flag   
            pACL, 
            FALSE))   // not a default DACL 
    {  
        return 0; 
    } 
 
    // Initialize a security attributes structure.
    sa.nLength = sizeof (SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = pSD;
    sa.bInheritHandle = FALSE;
	return 1;
}

BOOL WINAPI shutdown_handle(DWORD CtrlEvent) {
	ShutDown = TRUE;
	printf("In console control handler\n");
	Sleep(10);
	TerminateThread(hSrvrThread, 0);
	return TRUE;
}


DWORD WINAPI InstanceThread(LPVOID lpvParam){ 
	HANDLE hNamedPipe = (HANDLE) lpvParam; 
	DWORD nXfer;
	SearchRequest req;
	while (!ShutDown && ReadFile (hNamedPipe, &req, sizeof(SearchRequest), &nXfer, NULL)) { 
		process(req,hNamedPipe);
	}
	FlushFileBuffers(hNamedPipe); 
	DisconnectNamedPipe(hNamedPipe); 
	CloseHandle(hNamedPipe); 
	printf("InstanceThread exitting.\n");
	return 1;
}

DWORD WINAPI  start_named_pipe0(LPVOID lpvParam) { 
	init_right();
	while (!ShutDown) { 
		BOOL   fConnected = FALSE; 
		DWORD  dwThreadId = 0; 
		HANDLE hThread = NULL; 
		HANDLE hPipe = INVALID_HANDLE_VALUE;
		hPipe = CreateNamedPipe(SERVER_PIPE, PIPE_ACCESS_DUPLEX,
				PIPE_READMODE_MESSAGE | PIPE_TYPE_MESSAGE | PIPE_WAIT,
				PIPE_UNLIMITED_INSTANCES, 0, 0, 50 ,&sa);
		if (hPipe == INVALID_HANDLE_VALUE) {
			WIN_ERROR;
			break;
		}
		fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED); 
		if (fConnected){ 
			hThread = CreateThread(
				NULL,              // no security attribute 
				0,                 // default stack size 
				InstanceThread,    // thread proc
				(LPVOID) hPipe,    // thread parameter 
				0,                 // not suspended 
				&dwThreadId);      // returns thread ID 

			if (hThread == NULL) {
				break;
			}else{
				CloseHandle(hThread);
			}
		}else{
			CloseHandle(hPipe);// The client could not connect, so close the pipe. 
		}
	} 
	return 0; 
} 

int start_named_pipe() { 
	hSrvrThread = CreateThread(
		NULL,              // no security attribute 
		0,                 // default stack size 
		start_named_pipe0,    // thread proc
		NULL,    // thread parameter 
		0,                 // not suspended 
		NULL);      // returns thread ID 
	return hSrvrThread != NULL;

}

void wait_stop_named_pipe(){
	WaitForSingleObject(hSrvrThread,INFINITE);
	CloseHandle(hSrvrThread);
}
