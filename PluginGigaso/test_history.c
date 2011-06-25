#include <stdio.h>
#include "history.h"

void print(wchar_t *file, void *context){
	printf("%ls\n",file);
}

int main(){
	add_history(L"asdf");
	add_history(L"qwer");
	save_history();
	load_history();
	add_history(L"zxcv");
	save_history();
	load_history();
	HistoryIterator(print,NULL);
	{
		wchar_t buffer[MAX_HISTORY*MAX_PATH];
		int len = to_json(buffer);
		buffer[len+1] = L'\0';
		printf("%d,%ls\n",len,buffer);
	}
	return 0;
}

