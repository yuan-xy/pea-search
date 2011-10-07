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
#include "server.h"
#include "main.h"

static BOOL loaded_offline=0;

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
static void send_response_char(HANDLE hNamedPipe,char c){
	char buffer[8192], *p1=buffer+sizeof(int), *p=p1;
	*p++ = c;
	{
		DWORD nXfer;
		pSearchResponse resp = (pSearchResponse)buffer;
		resp->len = (p-p1);
		WriteFile (hNamedPipe, resp, (p-buffer), &nXfer, NULL);
	}
}

static void send_response_ok(HANDLE hNamedPipe){
	send_response_char(hNamedPipe,'1');
}

static void load_offline_dbs_t(void *p){
	load_offline_dbs();
}

static void rescan_t(void *p){
	rescan((int)p);
}
static BOOL update_status(int status){
	FILE *file;
	if ((file = fopen (UPDATE_CHECH_FILE, "w")) != NULL){
		fwrite(&status,sizeof(char),1,file);
		fclose (file);
		return 1;
	}
	return 0;
}

static int get_update_status(){
	BOOL one_day_ago = file_passed_one_day(UPDATE_CHECH_FILE);
	if(one_day_ago){
		return UPDATE_CHECH_UNKNOWN;
	}else{
		FILE *file;
		int status=UPDATE_CHECH_UNKNOWN;
		if ((file = fopen(UPDATE_CHECH_FILE, "r+")) == NULL){
			return UPDATE_CHECH_UNKNOWN;
		}
		fread(&status,sizeof(int),1,file);
		fclose (file);
		return status;
	}
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
			fwrite(&status,sizeof(char),1,file);
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
			update_status(UPDATE_CHECH_DONE);
			send_response_ok(hNamedPipe);
		}else if(wcsncmp(url,L"_status",wcslen(L"_status"))==0){
			send_response_char(hNamedPipe,get_update_status());
		}else{
			update_status(UPDATE_CHECH_DOWNLOADING);
			_beginthread(download_t,0,url);
			send_response_ok(hNamedPipe);
		}
	}else{
		send_response_ok(hNamedPipe);
	}
}

void process(SearchRequest req, void *out){
	HANDLE hNamedPipe = (HANDLE) out; 
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