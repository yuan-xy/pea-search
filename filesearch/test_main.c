#include "env.h"
#include "main.h"
#include "global.h"
#include "util.h"
#include "search.h"
#include "desktop.h"

void search0(void *name){
	pFileEntry *result=NULL;
	int count=0,i;
		NEW0(SearchEnv, sEnv);
		sEnv->case_sensitive=0;
		sEnv->order=0;
		sEnv->file_type=0;
		wcscpy(sEnv->path_name, L"E:\\backup");
		sEnv->path_len=0;
		{
			DWORD size=MAX_PATH;
			GetUserName(sEnv->user_name, &size);
		}
	count = search((WCHAR *)name,sEnv,&result);
	for(i=0;i<count;i++){
		pFileEntry file = *(result+i);
		FSIZE size;
		size = GET_SIZE(file);
		print_time(file);
		print_full_path(file);
		//PrintFilenameMB(file);
		printf("\n");
		if(i>30) break;
	}
	free_safe(sEnv);
	free_search(result);
}

int main(){
	WCHAR ss[64];
	gigaso_init();
	scan_desktop();
	do{
		//wscanf_s(L"%[^\n]",ss,31);
		fgetws(ss,63,stdin);
		if(ss[0]==L'q') break;
		printf("search : %d milli-seconds\n",time_passed_p1(search0,ss));
	}while(1);
	gigaso_destory();
	return 0;
}
