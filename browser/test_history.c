#include "history.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>


extern int n_index_form_w(int wi);
extern int w_index_from_n(int ni);

void print(TCHAR *file, void *context){
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
	history_add(__T("abcd"));
	assert( _tcscmp(history_get(0),__T("abcd"))==0 );
	history_add(__T("qwerrr"));
	assert( _tcscmp(history_get(0),__T("qwerrr"))==0 );
	history_add(__T("zxcv"));
	assert( _tcscmp(history_get(0),__T("zxcv"))==0 );
	history_save();
	history_load();
	history_add(__T("zxcv1"));
	assert( _tcscmp(history_get(0),__T("zxcv1"))==0 );
	assert( _tcscmp(history_get(1),__T("zxcv"))==0 );
	assert( _tcscmp(history_get(2),__T("qwerrr"))==0 );
	assert( _tcscmp(history_get(3),__T("abcd"))==0 );
	history_delete(1);
	history_save();
	history_load();
	assert( _tcscmp(history_get(0),__T("zxcv1"))==0 );
	assert( _tcscmp(history_get(1),__T("zxcv"))!=0 );
	assert( _tcscmp(history_get(2),__T("qwerrr"))==0 );
	assert( _tcscmp(history_get(3),__T("abcd"))==0 );
	history_pin(2);
	check_ni_wi();
	history_add(__T("asdff"));
	history_add(__T("asdfff"));
	history_add(__T("asdffff"));
	assert( _tcscmp(history_get(0),__T("asdffff"))==0 );
	assert( _tcscmp(history_get(1),__T("asdfff"))==0 );
	assert( _tcscmp(history_get(2),__T("qwerrr"))==0 );
	assert( _tcscmp(history_get(3),__T("asdff"))==0 );
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
		TCHAR buffer[VIEW_HISTORY*MAX_PATH];
		int len = history_to_json(buffer);
		buffer[len+1] = __T('\0');
		_tprintf(__T("\n%d,%s\n"),len,buffer);
	}
	return 0;
}

