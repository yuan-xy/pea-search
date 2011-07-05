#include <stdio.h>
#include "history.h"

void print(wchar_t *file, void *context){
	printf("%ls\n",file);
}

int main(){
	init_from_recent();
	history_add(L"asdf");
	history_add(L"qwer");
	history_save();
	history_load();
	history_add(L"zxcv");
	history_save();
	history_load();
	HistoryIterator(print,NULL);
	{
		wchar_t buffer[MAX_HISTORY*MAX_PATH];
		int len = history_to_json(buffer);
		buffer[len+1] = L'\0';
		printf("%d,%ls\n",len,buffer);
	}
	return 0;
}

