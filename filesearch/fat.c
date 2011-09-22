#include "env.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "suffix.h"
#include "search.h"
#include "global.h"
#include "util.h"
#include "fat.h"
#include "fs_common.h"

static const int ALL_LEN = 4;
static const WCHAR ALL[4]=L"*.*";
__declspec (thread) int count = 1;

/**
 * 根据文件查找结果新增一个pFileEntry，并挂载到父目录下。理论上支持所有文件系统类型。
 * @param pdf WIN32系统调用查找文件的结果
 * @param parent 在哪个目录下执行的查询
 */
static pFileEntry initFatFile(WIN32_FIND_DATA *pfd, pFileEntry parent, int i){
	int str_len = (int)wcslen(pfd->cFileName);
	int len = WCHAR_TO_UTF8_LEN(pfd->cFileName,str_len);
	NEW0_FILE(ret,len);
	ret->us.v.FileNameLength = len;
	ret->us.v.StrLen = str_len;
	WCHAR_TO_UTF8(pfd->cFileName,str_len,ret->FileName,len);
	if(is_dir_ffd(pfd)){
		ret->ut.v.suffixType = SF_DIR;
		ret->us.v.dir = 1;
	}
	if(is_readonly_ffd(pfd)) ret->us.v.readonly = 1;
	if(is_hidden_ffd(pfd)) ret->us.v.hidden = 1;
	if(is_system_ffd(pfd)) ret->us.v.system = 1;
	addChildren(parent,ret);
	SuffixProcess(ret,NULL);
	set_time(ret, &pfd->ftLastWriteTime);
	if(IsDir(ret)){
		SET_SIZE(ret,0);
	}else{
		LARGE_INTEGER filesize;
        filesize.LowPart = pfd->nFileSizeLow;
        filesize.HighPart = pfd->nFileSizeHigh;
		SET_SIZE(ret,file_size_shorten(filesize.QuadPart));
	}
	ALL_FILE_COUNT +=1;
	return ret;
}

void scanFolder(pFileEntry dir, WCHAR *full_name, int name_len, int i);

void scanFile(pFileEntry file,WCHAR *full_name, int name_len, int i){
	if(IsDir(file)){
		int wsize;
		WCHAR *filename = utf8_to_wchar(file->FileName,file->us.v.FileNameLength, &wsize);
		int new_name_len = name_len + (wsize + 1) * sizeof(WCHAR);
		WCHAR *new_full_name = (WCHAR *) malloc_safe(new_name_len);
		memcpy(new_full_name,full_name,name_len);
		memcpy(new_full_name+name_len/sizeof(WCHAR),filename,wsize * sizeof(WCHAR));
		*(new_full_name+new_name_len/sizeof(WCHAR)-1) = L'\\';
		scanFolder(file,new_full_name,new_name_len,i);
		free_safe(new_full_name);
		free_safe(filename);
	}
}

/**
 *
 * @param dir 在该目录下递归搜索所有文件
 * @param full_name 该目录的全路径名，路径名的最后一个字符是'\'，不包含'\0'
 * @param name_len 全路径名的字节长度，(是wcslen的两倍)
 */
static void scanFolder(pFileEntry dir, WCHAR *full_name, int name_len, int i) {
	WCHAR *pattern = (WCHAR *) malloc_safe(name_len + ALL_LEN*sizeof(WCHAR));
	WIN32_FIND_DATA fd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	memcpy(pattern,full_name,name_len);
	wcsncpy(pattern+name_len/sizeof(WCHAR),ALL,ALL_LEN);
	//wprintf(L"%s\n",pattern);
	hFind = FindFirstFile(pattern, &fd);
	if (INVALID_HANDLE_VALUE == hFind){
		FindClose(hFind);
		free_safe(pattern);
		return;
	}
	do{
		pFileEntry cur_file;
		if (fd.cFileName[0] == '.' && (fd.cFileName[1] == '\0' || fd.cFileName[1] == '.')) {
			continue;
		}
		cur_file = initFatFile(&fd, dir,i);
		count++;
	}while (FindNextFile(hFind, &fd) != 0);
	FindClose(hFind);
	free_safe(pattern);
	SubDirIterate_p3(dir,scanFile,full_name,name_len,i);
}



int scanRoot(pFileEntry dir, int i){
	WCHAR root[3];
	root[0]=dir->FileName[0];
	root[1]=dir->FileName[1];
	root[2]=L'\\';
	scanFolder(dir,root, 3*sizeof(WCHAR),i);
	printf("indexed %d\n",count);
	return count;
}

void OpenFatHandle(int i) {
	WCHAR name[4];
	HANDLE hFile;
	swprintf(name, 4, L"%c:\\", i + 'A');
	hFile = CreateFileW(name, FILE_LIST_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
			OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
			NULL);
	if (INVALID_HANDLE_VALUE == hFile) {
		WIN_ERROR
		return;
	}
	g_hVols[i] = hFile;
}

static void add_file(WCHAR *name, int len, int i){
	WIN32_FIND_DATA fd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	WCHAR *p = wcsrchr_me(name,len,L'\\');
	pFileEntry dir = find_file(name,p-name);
	if(dir==NULL) return;
	hFind = FindFirstFile(name, &fd);
	if (INVALID_HANDLE_VALUE == hFind){
		return;
	}
	initFatFile(&fd,dir,i);
}

static wchar_t RENAMED_OLD_NAME[MAX_PATH];

static DWORD WINAPI MonitorFat(PVOID pParam) {
	pFileEntry root = (pFileEntry)pParam;
    int i = getDrive(root);
	DWORD dwBytesReturned;
	size_t nBufSize = (sizeof(FILE_NOTIFY_INFORMATION) + MAX_PATH) * 2;
	FILE_NOTIFY_INFORMATION* pBuffer = (FILE_NOTIFY_INFORMATION*)calloc(1, nBufSize);
	FILE_NOTIFY_INFORMATION* pNotify;
	while (1) {
		BOOL flag = ReadDirectoryChangesW(
				g_hVols[i],
				pBuffer,
				nBufSize,
				TRUE,
				FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME, &dwBytesReturned, NULL,
				NULL);
		pNotify = pBuffer;
		while (flag && pNotify!=NULL) {
			if(pNotify->Action==FILE_ACTION_RENAMED_NEW_NAME){
				pFileEntry pmodify = find_file(RENAMED_OLD_NAME,wcslen(RENAMED_OLD_NAME));
				renameFile(pmodify ,pNotify->FileName,pNotify->FileNameLength);
			}else{
				int len = pNotify->FileNameLength+3*sizeof(WCHAR);
				int size = len/sizeof(WCHAR);
				if(size<MAX_PATH && size>0){
					WCHAR *name = (WCHAR *)malloc_safe(len);
					wsprintf(name,L"%c:\\%s",i+'A',pNotify->FileName);
					switch(pNotify->Action){
						case FILE_ACTION_ADDED: add_file(name,size,i);break;
						case FILE_ACTION_REMOVED: deleteFile(find_file(name,size)); break;
						case FILE_ACTION_RENAMED_OLD_NAME: {
							memset(RENAMED_OLD_NAME,0,sizeof(wchar_t)*MAX_PATH);
							memcpy(RENAMED_OLD_NAME,name,len); 
							break;
						}
					}
				}else{
					fprintf(stderr,"%s , line %d in '%s'\n",pNotify->FileName, __LINE__, __FILE__);
				}
			}
			if (pNotify->NextEntryOffset)
				pNotify = (FILE_NOTIFY_INFORMATION*)(((BYTE*)pNotify) + pNotify->NextEntryOffset);
			else
				pNotify = NULL;
		}
	}
}

BOOL StartMonitorThreadFAT(int i){
	if(g_hThread[i]!=NULL) return 0;
	g_hThread[i]=CreateThread(NULL,0,MonitorFat,g_rootVols[i],0,0);
	if(g_hThread[i]==NULL) return 0;
	return 1;
}
