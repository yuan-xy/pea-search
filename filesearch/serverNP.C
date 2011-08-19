#include "env.h"
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <process.h>
#include "util.h"
#include "common.h"
#include "global.h"
#include "drive.h"
#include "sharelib.h"
#include "search.h"
#include "suffix.h"
#include "write.h"
#include "serverNP.h"
#include "main.h"

typedef struct { /* Argument to a server thread. */
	HANDLE hNamedPipe; /* Named pipe instance. */
	unsigned int ThreadNo;
} THREAD_ARG;
typedef THREAD_ARG *LPTHREAD_ARG;

volatile static BOOL ShutDown = FALSE;
static unsigned int WINAPI Server (void *);
static unsigned int WINAPI Connect (void *);

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
}

/*  Force the connection thread to shut down if it is still active */
static void exit_conn_thread(HANDLE hConTh){
	DWORD ConThStatus;
	GetExitCodeThread (hConTh, &ConThStatus);
	if (ConThStatus == STILL_ACTIVE) {
		HANDLE hClient = CreateFile (SERVER_PIPE, GENERIC_READ | GENERIC_WRITE, 0, NULL,
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hClient != INVALID_HANDLE_VALUE) CloseHandle (hClient);
		if(WAIT_TIMEOUT==WaitForSingleObject (hConTh, 3*1000)){
			CloseHandle (hConTh);
		}
	}
}

static char * write(char *buffer, void *data, int size){
	char *p = (char *)data;
	int i;
	for(i=0;i<size;i++){
		*buffer = *p;
		buffer++; p++;
	}
	return buffer;
}


static char * write_file(char *buffer, pFileEntry file){
	char *p = buffer;
	*p++ = '{';
	{
		memcpy(p,"\"name\":\"",8);
		p += 8;
		memcpy(p,file->FileName,file->us.v.FileNameLength);
		p += file->us.v.FileNameLength;
		*p++ ='"';
		*p++ =',';
	}
	{
		memcpy(p,"\"path\":\"",8);
		p += 8;
		p += print_path_str(file, p);
		*p++ ='"';
		*p++ =',';
	}
	{
		memcpy(p,"\"type\":\"",8);
		p += 8;
		p += print_suffix_type(file, p);
		*p++ ='"';
		*p++ =',';
	}
	if(!IsDir(file)){
		FSIZE size = GET_SIZE(file);
		int sizea = file_size_amount(size);
		int sizeu = file_size_unit(size);
		memcpy(p,"\"size\":\"",8);
		p += 8;
		p += sprintf(p,"%d",sizea);
		switch(sizeu){
			case 1: memcpy(p," KB", 3);break;
			case 2: memcpy(p," MB", 3);break;
			case 3: memcpy(p," GB", 3);break;
			default:memcpy(p,"  B", 3);break;
		}
		p +=3;
		*p++ ='"';
		*p++ =',';
	}
	{
		memcpy(p,"\"time\":\"",8);
		p += 8;
		p += print_time_str(file,p);
		*p++ ='"';
	}
	*p++ = '}';
	return p;
}


static void send_response_search(HANDLE hNamedPipe, pSearchRequest req, pFileEntry *result, int count){
	char buffer[MAX_RESPONSE_LEN], *p1=buffer+sizeof(int), *p=p1;
	pFileEntry *start = result+req->from;
	int i;
	*p++ = '[';
	for(i=0;i<req->rows && i<count;i++){
		pFileEntry file = *(start+i);
		p = write_file(p,file);
		*p++ = ',';
		if((p-p1)>MAX_RESPONSE_LEN-100) break; //prevent buffer overflow
	}
	if(*(p-1)==',') p--;
	*p++ = ']';
	{
		DWORD nXfer;
		pSearchResponse resp = (pSearchResponse)buffer;
		resp->len = (p-p1);
		WriteFile (hNamedPipe, resp, (p-buffer), &nXfer, NULL);
	}
}

static void send_response_stat(HANDLE hNamedPipe, pSearchRequest req){
	char buffer[4096], *p1=buffer+sizeof(int), *p=p1;
	int *stats = stat(req->str, &(req->env) );
	p += print_stat(stats,p);
	{
		DWORD nXfer;
		pSearchResponse resp = (pSearchResponse)buffer;
		resp->len = (p-p1);
		WriteFile (hNamedPipe, resp, (p-buffer), &nXfer, NULL);
	}
}

static char * print_drive_info(char *buffer,int id){
	char *p = buffer;
	*p++ = '{';
	{
		memcpy(p,"\"id\":\"",6);
		p += 6;
		p += sprintf(p,"%d",id);
		*p++ ='"';
		*p++ =',';
	}
	{
		memcpy(p,"\"type\":\"",8);
		p += 8;
		p += sprintf(p,"%d",g_VolsInfo[id].type);
		*p++ ='"';
		*p++ =',';
	}
	{
		memcpy(p,"\"serialNumber\":\"",16);
		p += 16;
		p += sprintf(p,"%x",g_VolsInfo[id].serialNumber);
		*p++ ='"';
		*p++ =',';
	}
	if(wcslen(g_VolsInfo[id].volumeName)>0){
		pUTF8 str;
		int str_len;
		str = wchar_to_utf8(g_VolsInfo[id].volumeName,wcslen(g_VolsInfo[id].volumeName),&str_len);
		memcpy(p,"\"volumeName\":\"",14);
		p += 14;
		memcpy(p,str,str_len);
		free_safe(str);
		p += str_len;
		*p++ ='"';
		*p++ =',';
	}
	if(wcslen(g_VolsInfo[id].fsName)>0){
		pUTF8 str;
		int str_len;
		str = wchar_to_utf8(g_VolsInfo[id].fsName,wcslen(g_VolsInfo[id].fsName),&str_len);
		memcpy(p,"\"fsName\":\"",10);
		p += 10;
		memcpy(p,str,str_len);
		free_safe(str);
		p += str_len;
		*p++ ='"';
	}
	*p++ = '}';
	return p;
}

static void send_response_index_status(HANDLE hNamedPipe){
	char buffer[8192], *p1=buffer+sizeof(int), *p=p1;
	int i;
	*p++ = '[';
	for(i=0;i<DIRVE_COUNT_OFFLINE;i++){
		if(!g_loaded[i]) continue;
		p = print_drive_info(p,i);
		*p++ = ',';
		if((p-p1)>8192-100) break; //prevent buffer overflow
	}
	if(*(p-1)==',') p--;
	*p++ = ']';	
	{
		DWORD nXfer;
		pSearchResponse resp = (pSearchResponse)buffer;
		resp->len = (p-p1);
		WriteFile (hNamedPipe, resp, (p-buffer), &nXfer, NULL);
	}
}

static BOOL print_db_visitor(char *db_name, void *data){
		char **pp = (char **)data;
		char *p = *pp;
		*p++ = '"';
		memcpy(p,db_name,strlen(db_name));
		p += strlen(db_name);
		*p++ = '"';
		*p++ = ',';
		*pp = p;
		return 1;
}

static void send_response_cache_dbs(HANDLE hNamedPipe){
	char buffer[8192], *p1=buffer+sizeof(int), *p=p1;
	*p++ = '[';
	DbIterator(print_db_visitor,&p);
	if(*(p-1)==',') p--;
	*p++ = ']';	
	{
		DWORD nXfer;
		pSearchResponse resp = (pSearchResponse)buffer;
		resp->len = (p-p1);
		WriteFile (hNamedPipe, resp, (p-buffer), &nXfer, NULL);
	}
}

static void send_response_get_drives(HANDLE hNamedPipe){
	char buffer[8192], *p1=buffer+sizeof(int), *p=p1;
	int i;
	*p++ = '[';
	for(i=0;i<DIRVE_COUNT;i++){
		if(!g_bVols[i]) continue;
		p = print_drive_info(p,i);
		*p++ = ',';
		if((p-p1)>8192-100) break; //prevent buffer overflow
	}
	if(*(p-1)==',') p--;
	*p++ = ']';	
	{
		DWORD nXfer;
		pSearchResponse resp = (pSearchResponse)buffer;
		resp->len = (p-p1);
		WriteFile (hNamedPipe, resp, (p-buffer), &nXfer, NULL);
	}
}

static void send_response_rescan(HANDLE hNamedPipe){
	char buffer[8192], *p1=buffer+sizeof(int), *p=p1;
	*p++ = '1';
	{
		DWORD nXfer;
		pSearchResponse resp = (pSearchResponse)buffer;
		resp->len = (p-p1);
		WriteFile (hNamedPipe, resp, (p-buffer), &nXfer, NULL);
	}
}

static void command_exec(WCHAR *command, HANDLE hNamedPipe){
	if(wcsncmp(command,L"index_status",wcslen(L"index_status"))==0){
		send_response_index_status(hNamedPipe);
	}
	if(wcsncmp(command,L"cache_dbs",wcslen(L"cache_dbs"))==0){
		send_response_cache_dbs(hNamedPipe);
	}
	if(wcsncmp(command,L"get_drives",wcslen(L"get_drives"))==0){
		send_response_get_drives(hNamedPipe);
	}
	if(wcsncmp(command,L"rescan",wcslen(L"rescan"))==0){
		int i = *(command+wcslen(L"rescan")) - L'0';
		rescan(i);
		send_response_rescan(hNamedPipe);
	}
}

static BOOL loaded_offline=0;

static unsigned int WINAPI Server (void *pArg) {
	LPTHREAD_ARG pThArg = (LPTHREAD_ARG)pArg;
	HANDLE hNamedPipe= pThArg->hNamedPipe, hConTh = NULL;
	while (!ShutDown) {
		DWORD nXfer;
		SearchRequest req;
		hConTh = (HANDLE)_beginthreadex (NULL, 0, Connect, hNamedPipe, 0, NULL);
		if (hConTh == NULL) {
			WIN_ERROR;
			_endthreadex(2);
		}
		while (!ShutDown && WaitForSingleObject (hConTh, CS_TIMEOUT) == WAIT_TIMEOUT) {};
		if (ShutDown) continue;
		CloseHandle (hConTh); hConTh = NULL;
		while (!ShutDown && ReadFile (hNamedPipe, &req, sizeof(SearchRequest), &nXfer, NULL)) {
			if(req.env.offline && !loaded_offline){
				load_offline_dbs();
				loaded_offline=1;
			}
			if(req.rows==-1){
				send_response_stat(hNamedPipe, &req);
			}else if(wcsncmp(req.str,L"[///",4)==0){
				WCHAR *command = req.str + 4;
				command_exec(command, hNamedPipe);
			}else{
				pFileEntry *result=NULL;
				int count = search(req.str,&(req.env),&result);
				send_response_search(hNamedPipe,&req,result,count);
				free_search(result);
			}
		} /* Get next command */
		if(!FlushFileBuffers(hNamedPipe)) WIN_ERROR;
		if(!DisconnectNamedPipe(hNamedPipe)) WIN_ERROR;
	}
	if(hConTh!=NULL) exit_conn_thread(hConTh);
	printf("Exiting server thread number %d\n", pThArg->ThreadNo);
	_endthreadex (0);
	return 0;
}

static unsigned int WINAPI Connect (void *arg){
	BOOL f= ConnectNamedPipe ((HANDLE)arg, NULL);
	if(f){
		printf ("ConnNP finished: %d\n", f);
	}else{
		WIN_ERROR;
	}
	_endthreadex (0);
	return 0;
}

BOOL WINAPI shutdown_handle(DWORD CtrlEvent) {
	/* Shutdown the system */
	printf("In console control handler\n");
	ShutDown = TRUE;
	return TRUE;
}

void shutdown_NP(){
	int i;
	ShutDown = TRUE;
	Sleep(5*1000);
	for (i = 0; i < MAX_CLIENTS; i++) {
		if(ThArgs[i].hNamedPipe !=NULL) CloseHandle(ThArgs[i].hNamedPipe);
		if (hSrvrThread[i] != NULL) TerminateThread(hSrvrThread[i], 0);
	}
}

DWORD WINAPI  Hotkey(PVOID pParam){
    MSG msg = {0};
    if (!RegisterHotKey(NULL,1, 0, VK_PAUSE)) {	//0x42 is 'b'
        return 1;
    }
    wprintf(L"Hotkey 'ALT+b' registered, using MOD_NOREPEAT flag\n");
    while(GetMessage(&msg, NULL, 0, 0) != 0){
        if (msg.message == WM_HOTKEY){
			wchar_t buffer1[MAX_PATH], buffer2[MAX_PATH];
			wprintf(L"WM_HOTKEY received\n");
			get_abs_path(L"search.exe",buffer1);
			get_abs_path(L"",buffer2);
			{
				HINSTANCE  hi = ShellExecute(NULL, L"open", buffer1,NULL,buffer2,SW_SHOW); 
				DWORD ret = GetLastError();
				printf("%d",ret);
			}
		}
    }
	return 0;
}
