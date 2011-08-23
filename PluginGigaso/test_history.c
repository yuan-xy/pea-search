#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "history.h"

extern int n_index_form_w(int wi);
extern int w_index_from_n(int ni);

void print(wchar_t *file, void *context){
	printf("%ls\n",file);
}

void check_ni_wi(){
		int i;
		printf("\n");
		for(i=0;i<MAX_HISTORY;i++){
			printf("%d : %d : %d\n",i,n_index_form_w(i),w_index_from_n(n_index_form_w(i)));
			assert( w_index_from_n(n_index_form_w(i))==i );
		}
}

int main(){
	history_remove();
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
	check_ni_wi();
	history_add(L"asdff");
	history_add(L"asdfff");
	history_add(L"asdffff");
	assert( wcscmp(history_get(0),L"asdffff")==0 );
	assert( wcscmp(history_get(1),L"asdfff")==0 );
	assert( wcscmp(history_get(2),L"qwerrr")==0 );
	assert( wcscmp(history_get(3),L"asdff")==0 );
	history_pin(3);
	check_ni_wi();
	history_unpin(2);
	check_ni_wi();
	history_pin(11);
	history_pin(13);
	check_ni_wi();
	history_pin(5);
	history_pin(6);
	check_ni_wi();
	history_delete(4);
	history_delete(5);
	check_ni_wi();
	{
		wchar_t buffer[VIEW_HISTORY*MAX_PATH];
		int len = history_to_json(buffer);
		buffer[len+1] = L'\0';
		wprintf(L"\n%d,%s\n",len,buffer);
	}
	return 0;
}

