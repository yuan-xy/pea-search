#include <stdio.h>
#include <Shlobj.h>
#include "history.h"
#include "win_misc.h"

static wchar_t his_files[MAX_HISTORY][MAX_PATH];

//采用匈牙利命名法区分外部位置和内部位置
static int nstart=0; 

static struct{
	int wi:16; //外部位置
	int ni:16; //内部位置
}PIN[VIEW_HISTORY] = {-1}; //只初始化了第一个wi，是否存在移植性问题？默认初始化后，所有PIN[i].wi!=i

#define VALID_PIN(i) (i<VIEW_HISTORY && PIN[i].wi==i)

static int all_pined(){//所有被固定的记录的总数
	int i=0,ret=0;
	for(;i<VIEW_HISTORY;i++){
		if(VALID_PIN(i)) ret+=1;
	}
	return ret;
}
static BOOL occupy_by_pin(int ni){//给定位置是否被固定
	int i=0;
	for(;i<VIEW_HISTORY;i++){
		if(VALID_PIN(i)){
			if(ni==PIN[i].ni) return 1;
		}
	}
	return 0;
}

static void inc_start(){//前移首地址
	do{
		nstart++;
		if(MAX_HISTORY<=nstart) nstart=0;
	}while(occupy_by_pin(nstart));
}

static int n_index_form_w(int wi){//将外部地址转换为内部地址
	int i,nj,count=0,pin_under_wi=0;
	if(VALID_PIN(wi)) return PIN[wi].ni;
	for(i=0;i<wi;i++){
		if(VALID_PIN(i)) pin_under_wi++;
	}
	nj=nstart;
	while(1){
		nj-=1;
		if(nj<0) nj = MAX_HISTORY-1;
		if(!occupy_by_pin(nj)){
			if(count+pin_under_wi==wi) return nj;
			count++;
		}
	}
}

typedef struct{
	const wchar_t *filename;
	int flag;
} tmp_contain_context, *p_tmp_contain_context;

static void containVisitor(wchar_t *file, int pin, void *context){
	p_tmp_contain_context ctx = (p_tmp_contain_context)context;
	if(wcscmp(file,ctx->filename)==0) ctx->flag=1;
}

void history_add(const wchar_t *file){
	tmp_contain_context ctx;
	ctx.filename = file;
	ctx.flag = 0;
	HistoryIterator(containVisitor,&ctx);
	if(ctx.flag) return;
	wcscpy(his_files[nstart],file);
	inc_start();
}

void history_delete(int wi){
	int nj=nstart;
	do{
		nj+=1;
		if(nj>=MAX_HISTORY) nj=0;
	}while(occupy_by_pin(nj));
	memcpy(his_files[n_index_form_w(wi)],his_files[nj], sizeof(his_files[0]));
	wcscpy(his_files[nj],L"");
}

void history_pin(int wi){
	if(wi>=0 && wi<VIEW_HISTORY){
		int ni = n_index_form_w(wi);
		PIN[wi].ni = ni;
		PIN[wi].wi = wi;
	}
}

wchar_t *history_get(int wi){
	return his_files[n_index_form_w(wi)];
}

BOOL history_save(){
	FILE *fp;
	fp = fopen("history.ini", "wb");
	if(fp==NULL) return 0;
	fwrite(&nstart,sizeof(int),1,fp);
	fwrite(PIN,sizeof(PIN[0]),VIEW_HISTORY,fp);
	fwrite(his_files,sizeof(his_files[0]),MAX_HISTORY,fp);
	fclose(fp);
	return 1;
}

BOOL history_load(){
	FILE *fp;
	fp = fopen("history.ini", "rb");//采用二进制流
	//unicode编码的中文“业”的hex值为“4e 1a”，其中1a是Ctrl-Z，被windows认为是文件结束标志。
	if(fp==NULL){
		init_from_recent();
		return 0;
	}
	fread(&nstart,sizeof(int),1,fp);
	fread(PIN,sizeof(PIN[0]),VIEW_HISTORY,fp);
	fread(his_files,sizeof(his_files[0]),MAX_HISTORY,fp);
	fclose(fp);
	return 1;
}

void HistoryIterator(pHistoryVisitor visitor, void *context){
	int i;
	for(i=0;i<VIEW_HISTORY;i++){
		int nj = n_index_form_w(i);
		(*visitor)(his_files[nj], VALID_PIN(i),context);
	}
}

void HistoryIteratorAll(pHistoryVisitor visitor, void *context){
	int i;
	for(i=0;i<MAX_HISTORY;i++){
		int nj = n_index_form_w(i);
		(*visitor)(his_files[nj], VALID_PIN(i), context);
	}
}

typedef struct{
	wchar_t *p;
} tmp_json_context, *p_tmp_json_context;

static void json_print(wchar_t *file, int pin, void *context){
	p_tmp_json_context pctx = (p_tmp_json_context)context;
	wchar_t *p = pctx->p;
	*p++ = L'{';
	{
		memcpy(p,L"\"name\":\"",8*sizeof(wchar_t));
		p += 8;
		wcscpy(p,file);
		p+=wcslen(file);
		*p++ =L'"';
		*p++ =L',';
	}
	{
		memcpy(p,L"\"pin\":",6*sizeof(wchar_t));
		p += 6;
		p += wsprintf(p,L"%d",pin);
	}
	*p++ = L'}';
	*p++ = L',';
	pctx->p = p;
}

int history_to_json(wchar_t *buffer){
	wchar_t *p = buffer;
	tmp_json_context ctx;
	*p++ = L'[';
	ctx.p = p;
	HistoryIteratorAll(json_print,&ctx);
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
			for(;i<MAX_HISTORY;i++){
				shortcut(list[i].path, list[i].path);
				history_add(list[i].path);
			}
			//nstart = MAX_HISTORY/2 +1;
			history_save();
		}
		FindClose(hFind);
	}
}
