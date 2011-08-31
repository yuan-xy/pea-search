#include "env.h"
#include <Urlmon.h>
#include <Wininet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <aclapi.h>
#include <time.h>
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

static BOOL loaded_offline=0;

typedef struct { /* Argument to a server thread. */
	HANDLE hNamedPipe; /* Named pipe instance. */
	unsigned int ThreadNo;
	HANDLE connThread;
} THREAD_ARG;
typedef THREAD_ARG *LPTHREAD_ARG;

volatile static BOOL ShutDown = FALSE;
static unsigned int WINAPI Server (void *);
static unsigned int WINAPI Connect (void *);

static HANDLE hSrvrThread[MAX_CLIENTS];
static THREAD_ARG ThArgs[MAX_CLIENTS];

static PSID pEveryoneSID[MAX_CLIENTS] = {}, pAdminSID[MAX_CLIENTS] = {};
static PACL pACL[MAX_CLIENTS] = {};
static PSECURITY_DESCRIPTOR pSD[MAX_CLIENTS] = {};
static EXPLICIT_ACCESS ea[MAX_CLIENTS][2];
static SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
static SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
static SECURITY_ATTRIBUTES sa[MAX_CLIENTS];

static BOOL init_right(int i){
    if(!AllocateAndInitializeSid(&SIDAuthWorld, 1,
                     SECURITY_WORLD_RID,
                     0, 0, 0, 0, 0, 0, 0,
                     &pEveryoneSID[i]))
    {
        return 0; 
    }
 
    ZeroMemory(&ea[i], 2 * sizeof(EXPLICIT_ACCESS));
    ea[i][0].grfAccessPermissions = GENERIC_READ | GENERIC_WRITE;
    ea[i][0].grfAccessMode = SET_ACCESS;
    ea[i][0].grfInheritance= NO_INHERITANCE;
    ea[i][0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[i][0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea[i][0].Trustee.ptstrName  = (LPTSTR) pEveryoneSID[i];

    if(! AllocateAndInitializeSid(&SIDAuthNT, 2,
                     SECURITY_BUILTIN_DOMAIN_RID,
                     DOMAIN_ALIAS_RID_ADMINS,
                     0, 0, 0, 0, 0, 0,
                     &pAdminSID[i])) 
    {
        return 0; 
    }
 
    ea[i][1].grfAccessPermissions = GENERIC_ALL | KEY_ALL_ACCESS;
    ea[i][1].grfAccessMode = SET_ACCESS;
    ea[i][1].grfInheritance= NO_INHERITANCE;
    ea[i][1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[i][1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ea[i][1].Trustee.ptstrName  = (LPTSTR) pAdminSID[i];

    if (ERROR_SUCCESS != SetEntriesInAcl(2, ea[i], NULL, &pACL[i])) 
    {
        return 0; 
    }
 
    // Initialize a security descriptor.  
    pSD[i] = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, 
                             SECURITY_DESCRIPTOR_MIN_LENGTH); 
    if (NULL == pSD[i]) 
    { 
        return 0; 
    } 
 
    if (!InitializeSecurityDescriptor(pSD[i],
            SECURITY_DESCRIPTOR_REVISION)) 
    {  
        return 0; 
    } 
 
    // Add the ACL to the security descriptor. 
    if (!SetSecurityDescriptorDacl(pSD[i], 
            TRUE,     // bDaclPresent flag   
            pACL[i], 
            FALSE))   // not a default DACL 
    {  
        return 0; 
    } 
 
    // Initialize a security attributes structure.
    sa[i].nLength = sizeof (SECURITY_ATTRIBUTES);
    sa[i].lpSecurityDescriptor = pSD[i];
    sa[i].bInheritHandle = FALSE;
	return 1;
}


BOOL start_named_pipe(){
	int i;
	for (i = 0; i < MAX_CLIENTS; i++) {
		init_right(i);
		HANDLE hNp = CreateNamedPipe(SERVER_PIPE, PIPE_ACCESS_DUPLEX,
				PIPE_READMODE_MESSAGE | PIPE_TYPE_MESSAGE | PIPE_WAIT,
				MAX_CLIENTS, 0, 0, 50 ,&sa[i]);
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
		TerminateThread(hConTh,0);
		return;
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
	int *stats = statistic(req->str, &(req->env) );
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
	{
		memcpy(p,"\"totalMB\":\"",11);
		p += 11;
		p += sprintf(p,"%d",g_VolsInfo[id].totalMB);
		*p++ ='"';
		*p++ =',';
	}
	{
		memcpy(p,"\"totalFreeMB\":\"",15);
		p += 15;
		p += sprintf(p,"%d",g_VolsInfo[id].totalFreeMB);
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
	*p++ = '{';
	{
		memcpy(p,"\"name\":\"",8);
		p += 8;
		memcpy(p,db_name,strlen(db_name));
		p += strlen(db_name);
		*p++ ='"';
		*p++ =',';
	}
	{
		memcpy(p,"\"time\":\"",8);
		p += 8;
		{
			struct stat statbuf;
			stat(db_name, &statbuf);
			//p += sprintf(p,"%s",ctime(&statbuf.st_mtime));
			p += sprintf(p,"%d",statbuf.st_mtime);
		}
		*p++ ='"';
	}
	*p++ = '}';
	*p++ =',';
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

static void send_response_ok(HANDLE hNamedPipe){
	char buffer[8192], *p1=buffer+sizeof(int), *p=p1;
	*p++ = '1';
	{
		DWORD nXfer;
		pSearchResponse resp = (pSearchResponse)buffer;
		resp->len = (p-p1);
		WriteFile (hNamedPipe, resp, (p-buffer), &nXfer, NULL);
	}
}

static void load_offline_dbs_t(void *p){
	load_offline_dbs();
}

static void rescan_t(void *p){
	rescan((int)p);
}

static void download_t(void *str){// "http://host/filename?hash&version"
	HRESULT hr;
	WCHAR *filename;
	WCHAR *url =(WCHAR *)str;
	WCHAR *p = wcsrchr(url,L'?');
	WCHAR *p2 = wcsrchr(url,L'&');
	WCHAR *hash = p+1;
	WCHAR *version = p2+1;
	*p = L'\0';
	*p2 = L'\0';
	filename = wcsrchr(url,L'/')+1;
	DeleteUrlCacheEntry(url);
	hr = URLDownloadToFile(NULL,url,filename,0,NULL);
	if(hr==S_OK){
		char md5_2[MD5_LEN*2+1];
		char fname[MAX_PATH]={0};
		char md5[MAX_PATH];
		WideCharToMultiByte(CP_ACP, 0, filename, wcslen(filename), fname, wcslen(filename), NULL, NULL);
		WideCharToMultiByte(CP_ACP, 0, hash, wcslen(hash), md5, MAX_PATH, NULL, NULL);		
		MD5File(fname,md5_2);
		if(strncmp(md5,md5_2,MD5_LEN*2)==0){
			int status=UPDATE_CHECH_NEW;
			FILE *file;
			if ((file = fopen (UPDATE_CHECH_FILE, "w")) == NULL) return;
			fwrite(&status,sizeof(int),1,file);
			fwrite(fname,sizeof(char),MAX_PATH,file);
			fclose (file);
			set_prop(L"version",version);
		}else{
			printf("hash %s != %s.\n",md5,md5_2);
		}
	}
}

static void command_exec(WCHAR *command, HANDLE hNamedPipe){
	if(wcsncmp(command,L"index_status",wcslen(L"index_status"))==0){
		send_response_index_status(hNamedPipe);
	}else if(wcsncmp(command,L"cache_dbs",wcslen(L"cache_dbs"))==0){
		send_response_cache_dbs(hNamedPipe);
	}else if(wcsncmp(command,L"get_drives",wcslen(L"get_drives"))==0){
		send_response_get_drives(hNamedPipe);
	}else if(wcsncmp(command,L"load_offline_db",wcslen(L"load_offline_db"))==0){
		if(!loaded_offline){
			loaded_offline=1;
			_beginthread(load_offline_dbs_t,0,NULL);
		}
		send_response_ok(hNamedPipe);
	}else if(wcsncmp(command,L"rescan",wcslen(L"rescan"))==0){
		int i = *(command+wcslen(L"rescan")) - L'0';
		_beginthread(rescan_t,0,(void *)i);
		send_response_ok(hNamedPipe);
	}else if(wcsncmp(command,L"del_offline_db",wcslen(L"del_offline_db"))==0){
		int i0 = *(command+wcslen(L"del_offline_db")) - L'0';
		int i1 = *(command+wcslen(L"del_offline_db?")) - L'0';
		del_offline_db(i0*10+i1);
		send_response_ok(hNamedPipe);
	}else if(wcsncmp(command,L"upgrade",wcslen(L"upgrade"))==0){
		WCHAR *url = command+wcslen(L"upgrade");
		if(wcsncmp(url,L"_none",wcslen(L"_none"))==0){
			FILE *file;
			if ((file = fopen (UPDATE_CHECH_FILE, "w")) != NULL){
				int status = UPDATE_CHECH_DONE;
				fwrite(&status,sizeof(int),1,file);
				fclose (file);
			}
		}else{
			_beginthread(download_t,0,url);
		}
		send_response_ok(hNamedPipe);
	}else{
		send_response_ok(hNamedPipe);
	}
}

static unsigned int WINAPI Server (void *pArg) {
	LPTHREAD_ARG pThArg = (LPTHREAD_ARG)pArg;
	HANDLE hNamedPipe= pThArg->hNamedPipe;
	while (!ShutDown) {
		DWORD nXfer;
		SearchRequest req;
		pThArg->connThread = (HANDLE)_beginthreadex (NULL, 0, Connect, hNamedPipe, 0, NULL);
		if (pThArg->connThread == NULL) {
			WIN_ERROR;
			_endthreadex(2);
		}
		while (!ShutDown && WaitForSingleObject (pThArg->connThread, CS_TIMEOUT) == WAIT_TIMEOUT) {};
		if (ShutDown) break;
		CloseHandle (pThArg->connThread);
		while (!ShutDown && ReadFile (hNamedPipe, &req, sizeof(SearchRequest), &nXfer, NULL)) {
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
	if(pThArg->connThread!=NULL) exit_conn_thread(pThArg->connThread);
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
	shutdown_NP();
	return TRUE;
}

void shutdown_NP(){
	int i;
	ShutDown = TRUE;
	for (i = 0; i < MAX_CLIENTS; i++) {
		if(ThArgs[i].connThread !=NULL) TerminateThread(ThArgs[i].connThread, 0);
		if (hSrvrThread[i] != NULL) TerminateThread(hSrvrThread[i], 0);
		if(ThArgs[i].hNamedPipe !=NULL){
			DisconnectNamedPipe(ThArgs[i].hNamedPipe);//这条语句必须被执行，否则下次无法正常CreateNamedPipe。
			CloseHandle(ThArgs[i].hNamedPipe);
		}
	}
}

/**
*不限制连接数，当有新连接时启动新线程。
*本模式的问题在于：当要求服务器强制退出时，要关闭所有打开过的线程和pipe，而无法得知那些线程/pipe已无效。
DWORD WINAPI InstanceThread(LPVOID lpvParam){ 
	HANDLE hNamedPipe = (HANDLE) lpvParam; 
	DWORD nXfer;
	SearchRequest req;
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
	}
	FlushFileBuffers(hNamedPipe); 
	DisconnectNamedPipe(hNamedPipe); 
	CloseHandle(hNamedPipe); 
	printf("InstanceThread exitting.\n");
	return 1;
}

int _tmain0(VOID) { 
	HANDLE hPipe = INVALID_HANDLE_VALUE, hThread = NULL; 
	while (!ShutDown) { 
		BOOL   fConnected = FALSE; 
		DWORD  dwThreadId = 0; 
		HANDLE hNp = CreateNamedPipe(SERVER_PIPE, PIPE_ACCESS_DUPLEX,
				PIPE_READMODE_MESSAGE | PIPE_TYPE_MESSAGE | PIPE_WAIT,
				PIPE_UNLIMITED_INSTANCES, 0, 0, 50 ,NULL);
		if (hNp == INVALID_HANDLE_VALUE) {
			WIN_ERROR;
			return 0;
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
				return -1;
			}else{
				;;
				CloseHandle(hThread); 
			}
		}else{
			// The client could not connect, so close the pipe. 
			CloseHandle(hPipe); 
		}
	} 
	return 0; 
} 
*/