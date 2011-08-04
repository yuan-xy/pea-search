#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "history.h"

void print(wchar_t *file, void *context){
	printf("%ls\n",file);
}

int main(){
	system("del history.ini");
	history_load();
	history_add(L"abcd");
	assert( wcscmp(history_get(0),L"abcd")==0 );
	history_add(L"qwerrr");
	assert( wcscmp(history_get(0),L"qwerrr")==0 );
	history_add(L"zxcv");
	assert( wcscmp(history_get(0),L"zxcv")==0 );
	history_save();
	history_load();
	history_add(L"zxcv1");
	assert( wcscmp(history_get(0),L"zxcv1")==0 );
	assert( wcscmp(history_get(1),L"zxcv")==0 );
	assert( wcscmp(history_get(2),L"qwerrr")==0 );
	assert( wcscmp(history_get(3),L"abcd")==0 );
	history_delete(1);
	history_save();
	history_load();
	assert( wcscmp(history_get(0),L"zxcv1")==0 );
	assert( wcscmp(history_get(1),L"zxcv")!=0 );
	assert( wcscmp(history_get(2),L"qwerrr")==0 );
	assert( wcscmp(history_get(3),L"abcd")==0 );
	history_pin(2);
	history_add(L"asdff");
	history_add(L"asdfff");
	history_add(L"asdffff");
	assert( wcscmp(history_get(0),L"asdffff")==0 );
	assert( wcscmp(history_get(1),L"asdfff")==0 );
	assert( wcscmp(history_get(2),L"qwerrr")==0 );
	assert( wcscmp(history_get(3),L"asdff")==0 );
	{
		wchar_t buffer[VIEW_HISTORY*MAX_PATH];
		int len = history_to_json(buffer);
		buffer[len+1] = L'\0';
		printf("%d,%ls\n",len,buffer);
	}
	return 0;
}

