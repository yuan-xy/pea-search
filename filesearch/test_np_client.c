#include "env.h"
#include "../filesearch/sharelib.h"
#include <stdio.h>

#define WIN_ERROR fprintf(stderr,"error code : %d , line %d in '%s'\n",GetLastError(), __LINE__, __FILE__);

static BOOL connect_named_pipe(HANDLE *p){
	HANDLE handle;
	WaitNamedPipe(SERVER_PIPE, NMPWAIT_WAIT_FOREVER);
	handle = CreateFile(SERVER_PIPE, GENERIC_READ | GENERIC_WRITE, 0,
			NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE) {
		WIN_ERROR;
		return 0;
	}else{
		*p = handle;
		return 1;
	}
}

int main(int argc, LPTSTR argv[]) {
	HANDLE hNamedPipe;
	SearchRequest req;
	SearchResponse resp;
	DWORD nRead, nWrite;
	memset(&req,0,sizeof(SearchRequest));
	req.from = 0;
	req.rows = 3;
	if(connect_named_pipe(&hNamedPipe)){
		do{
			fgetws(req.str,63,stdin);
			if(req.str[0]==L'q') break;
			if (!WriteFile(hNamedPipe, &req, sizeof(SearchRequest), &nWrite, NULL)) {
				WIN_ERROR;
				return 1;
			}
			if(ReadFile(hNamedPipe, &resp, sizeof(int), &nRead, NULL)){
				char buffer[1024];
				DWORD len = resp.len;
				printf("%d,", len);
				memset(buffer,(char)0,1024);
				ReadFile(hNamedPipe, buffer, len, &nRead, NULL);
				if(nRead!=len){
					printf("error\n");
				}
				printf("%s\n", buffer);
			}
		}while(1);
		CloseHandle(hNamedPipe);
	}
	return 0;
}

