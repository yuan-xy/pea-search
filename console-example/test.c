#define UNICODE
#define _UNICODE
#include "../filesearch/sharelib.h"

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void search0(void *name){
	pFileEntry *result=NULL;
	int count=0,i;
		pSearchEnv  sEnv = (pSearchEnv)malloc(sizeof(SearchEnv));
		sEnv->case_sensitive=0;
		sEnv->order=0;
		sEnv->file_type=0;
		sEnv->path_name = L"c:/windows";
		sEnv->path_len=0;
	count = search((WCHAR *)name,sEnv,&result);
	for(i=0;i<count;i++){
		pFileEntry file = *(result+i);
		FSIZE size;
		//size = GET_SIZE(file);
		//print_full_path(file);
		//printf("\n");
		if(i>60) break;
	}
	free(sEnv);
}

int time_passed_p1(void (*f)(void *),void *p1){
#ifdef WIN32
	int end,start = GetTickCount();
	(*f)(p1);
	end = GetTickCount();
	return end-start;
#else
	time_t start,end;
	start = time(NULL);
	if(start == (time_t)-1) return -1;
	(*f)(p1);
	end = time(NULL);
	return (int)(end-start)*1000;
#endif
}

int main(){
	WCHAR ss[64];
	gigaso_init();
	do{
		//wscanf_s(L"%[^\n]",ss,31);
		fgetws(ss,63,stdin);
		if(ss[0]==L'q') break;
		printf("time: %d\n",time_passed_p1(search0,ss));
	}while(1);
	gigaso_destory();
	return 0;
}
