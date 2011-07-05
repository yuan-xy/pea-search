#include <stdio.h>
#include <Shlobj.h>
#include "history.h"

static wchar_t his_files[MAX_HISTORY][MAX_PATH];
static int start=0;

static void inc_start(){
	start++;
	if(MAX_HISTORY<=start) start=0;
}

static int index_transform(int i){
	int j = start-i-1;
	if(j<0) j += MAX_HISTORY;
	return j;
}

void history_add(const wchar_t *file){
	wcscpy(his_files[start],file);
	inc_start();
}

void history_delete(int i){
	int j;
	for(j=0;j<MAX_HISTORY/2;j++){
		if(wcscmp(his_files[j],L"")!=0){
			memcpy(his_files[index_transform(i)],his_files[j], sizeof(his_files[0]));
			wcscpy(his_files[j],L"");
			return;
		}
	}
	wcscpy(his_files[index_transform(i)],L"");
}

BOOL history_save(){
	FILE *fp;
	fp = fopen("history.ini", "w");
	if(fp==NULL) return 0;
	fwrite(&start,sizeof(int),1,fp);
	fwrite(his_files,sizeof(his_files[0]),MAX_HISTORY,fp);
	fclose(fp);
	return 1;
}

BOOL history_load(){
	FILE *fp;
	fp = fopen("history.ini", "r");
	if(fp==NULL){
		init_from_recent();
		return 0;
	}
	fread(&start,sizeof(int),1,fp);
	fread(his_files,sizeof(his_files[0]),MAX_HISTORY,fp);
	fclose(fp);
	return 1;
}

void HistoryIterator(pHistoryVisitor visitor, void *context){
	int i;
	for(i=0;i<MAX_HISTORY;i++){
		int j = index_transform(i);
		(*visitor)(his_files[j],context);
	}
}

typedef struct{
	wchar_t *p;
} tmp_json_context, *p_tmp_json_context;

static void json_print(wchar_t *file, void *context){
	p_tmp_json_context pctx = (p_tmp_json_context)context;
	wchar_t *p = pctx->p;
	*p++ = L'"';
	wcscpy(p,file);
	p+=wcslen(file);
	*p++ = L'"';
	*p++ = L',';
	pctx->p = p;
}

int history_to_json(wchar_t *buffer){
	wchar_t *p = buffer;
	tmp_json_context ctx;
	*p++ = L'[';
	ctx.p = p;
	HistoryIterator(json_print,&ctx);
	p = ctx.p;
	*(p-1) = L']';
	*p = L'\0';
	return p-buffer;
}

#define MAX_FILE_STRUCT 1000
static struct file_tag{
	WCHAR path[MAX_PATH];
	FILETIME ftLastWriteTime;
};
typedef struct file_tag file, *pfile;

static int recent_compare(const void *pa, const void *pb){
	pfile a, b;
	a = (pfile)pa;
	b = (pfile)pb;
	if(b->ftLastWriteTime.dwHighDateTime == a->ftLastWriteTime.dwHighDateTime){
		return b->ftLastWriteTime.dwLowDateTime - a->ftLastWriteTime.dwLowDateTime;
	}else{
		return b->ftLastWriteTime.dwHighDateTime - a->ftLastWriteTime.dwHighDateTime;
	}
}

void init_from_recent(){
	WCHAR szPath[MAX_PATH];
	file list[MAX_FILE_STRUCT];
	if(SUCCEEDED(SHGetFolderPath(NULL, 
								 CSIDL_RECENT, 
								 NULL, 
								 0, 
								 szPath))) 
	{
		int count=0;
		WIN32_FIND_DATA fd;
		HANDLE hFind = INVALID_HANDLE_VALUE;
		int len = wcslen(szPath);
		szPath[len] = L'\\';
		szPath[len+1] = L'*';
		szPath[len+2] = L'\0';
		hFind = FindFirstFile(szPath, &fd);
		if (INVALID_HANDLE_VALUE == hFind){
			FindClose(hFind);
			return;
		}
		do{
			if (fd.cFileName[0] == '.' && (fd.cFileName[1] == '\0' || fd.cFileName[1] == '.')) {
				continue;
			}
			memcpy(list[count].path,szPath, sizeof(szPath));
			wcscpy(list[count].path+len+1,fd.cFileName);
			list[count].ftLastWriteTime = fd.ftLastWriteTime;
			printf("%d - %ls\n",count, list[count].path);
			count++;
			if(count >= MAX_FILE_STRUCT) break;
		}while (FindNextFile(hFind, &fd) != 0);
		qsort(list,count,sizeof(list[0]),recent_compare);
		{
			int i=0;
			for(;i<MAX_HISTORY;i++) history_add(list[i].path);
			start = MAX_HISTORY/2 +1;
		}
		FindClose(hFind);
	}
}