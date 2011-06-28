#include <stdio.h>
#include "history.h"

static wchar_t his_files[MAX_HISTORY][MAX_PATH];
static int start=0;

void history_add(const wchar_t *file){
	wcscpy(his_files[start],file);
	start++;
}

BOOL history_save(){
	FILE *fp;
	fp = fopen("history.ini", "w");
	if(fp==NULL) return 0;
	fwrite(&start,sizeof(int),1,fp);
	fwrite(his_files,MAX_PATH,MAX_HISTORY,fp);
	fclose(fp);
	return 1;
}

BOOL history_load(){
	FILE *fp;
	fp = fopen("history.ini", "r");
	if(fp==NULL) return 0;
	fread(&start,sizeof(int),1,fp);
	fread(his_files,MAX_PATH,MAX_HISTORY,fp);
	fclose(fp);
	return 1;
}

void HistoryIterator(pHistoryVisitor visitor, void *context){
	int i;
	for(i=0;i<MAX_HISTORY;i++){
		int j = start-i-1;
		if(j<0) j += MAX_HISTORY;
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
