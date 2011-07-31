#include <windows.h>
#include <stdio.h>

int main(int argc, char *argv[]){
	SC_HANDLE hScm = OpenSCManager (NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hScm == NULL) return 1;
	if(argc==1){
		printf("usage: %s [create path_to_exe] | delete | start",argv[0]);
	}else if(argc==2){
		if(strcmp("delete",argv[1])==0){
			SC_HANDLE hSc = OpenServiceA(hScm, "gigaso", DELETE);
			if (hSc == NULL) return 3;
			if (!DeleteService (hSc)) return 3;
			CloseServiceHandle (hSc);
			return 0;
		}
		if(strcmp("start",argv[1])==0){
			SC_HANDLE hSc = OpenServiceA(hScm,  "gigaso", SERVICE_ALL_ACCESS);
			if (hSc == NULL) return 4;
			if (!StartService (hSc, 0, NULL)) return 4;
			CloseServiceHandle (hSc);
			return 0;
		}
		printf("usage: %s [create path_to_exe] | delete | start",argv[0]);
	}else{
		if(strcmp("create",argv[1])==0){
			SC_HANDLE hSc = CreateServiceA (hScm, "gigaso", "Gigaso File Searcher",
				SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
				SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
				argv[2], NULL, NULL, NULL, NULL, NULL);
			if (hSc == NULL) return 2;
			else CloseServiceHandle (hSc);
			return 0;
		}
	}
	return 0;
}
