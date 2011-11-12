#include "env.h"
#include "main.h"
#include "global.h"
#include "util.h"
#include "server.h"


void search0(void *name){
	SearchRequest req={0};
    req.from=1;
    req.rows=1;
    wcscpy(req.str,name);
	process(req, STDERR_FILENO);
}

int main(){
	WCHAR ss[64];
	gigaso_init();
	do{
		fgetws(ss,63,stdin);
		if(ss[0]==L'q') break;
        {
            wchar_t *end = ss+wcslen(ss)-1;
            if(*end==L'\n') *end=L'\0';
        }
		printf("search : %d milli-seconds\n",time_passed_p1(search0,ss));
	}while(1);
	gigaso_destory();
	return 0;
}
