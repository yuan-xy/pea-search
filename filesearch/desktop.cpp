#include "env.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "suffix.h"
#include "search.h"
#include "global.h"
#include "util.h"
#include "fs_common.h"
#include "write.h"

#include <shlobj.h>
#include <shlwapi.h>
#include <iostream>

#include <google/sparse_hash_map>

static int depth=0;

enum FileType { LOCAL, NETWORK, VIRTUAL };

extern "C" {

BOOL ExecuteFileAction(HWND hwnd,TCHAR *szVerb,TCHAR *szStartDirectory,LPCITEMIDLIST pidl)
{
	SHELLEXECUTEINFO ExecInfo;

	ExecInfo.cbSize			= sizeof(SHELLEXECUTEINFO);
	ExecInfo.fMask			= SEE_MASK_INVOKEIDLIST;
	ExecInfo.lpVerb			= szVerb;
	ExecInfo.lpIDList		= (LPVOID)pidl;
	ExecInfo.hwnd			= hwnd;
	ExecInfo.nShow			= SW_SHOW;
	ExecInfo.lpParameters	= L"";
	ExecInfo.lpDirectory	= szStartDirectory;
	ExecInfo.lpFile			= NULL;
	ExecInfo.hInstApp		= NULL;

	return ShellExecuteEx(&ExecInfo);
}

HRESULT GetDisplayName(LPITEMIDLIST pidlDirectory,TCHAR *szDisplayName,DWORD uFlags)
{
	IShellFolder	*pShellFolder = NULL;
	LPITEMIDLIST	pidlRelative = NULL;
	STRRET			str;
	HRESULT			hr;

	hr = SHBindToParent(pidlDirectory,IID_IShellFolder,(void **)&pShellFolder,
	(LPCITEMIDLIST *)&pidlRelative);

	if(SUCCEEDED(hr))
	{
		hr = pShellFolder->GetDisplayNameOf(pidlRelative,uFlags,&str);

		if(SUCCEEDED(hr))
		{
			StrRetToBuf(&str,pidlDirectory,szDisplayName,MAX_PATH);
		}

		pShellFolder->Release();
	}

	return hr;
}

static void open_pidl0(LPITEMIDLIST pidlItem){
	TCHAR			szItemDirectory[MAX_PATH];
	LPITEMIDLIST	pidlParent = NULL;
	pidlParent = ILClone(pidlItem);
	ILRemoveLastID(pidlParent);
	GetDisplayName(pidlParent,szItemDirectory,SHGDN_FORPARSING);
	ExecuteFileAction(NULL,L"",szItemDirectory,(LPCITEMIDLIST)pidlItem);
	CoTaskMemFree(pidlParent);
}

static void open_pidl(LPITEMIDLIST	pidl, LPITEMIDLIST	ridl){
	if(pidl==NULL){
		open_pidl0(ridl);
	}else{
		LPITEMIDLIST	pidlComplete = NULL;
		pidlComplete = ILCombine(pidl,ridl);
		open_pidl0(pidlComplete);
		CoTaskMemFree(pidlComplete);
	}
}

static pFileEntry genDesktopFileEntry(){
	NEW0(FileEntry, ret);
	ret->FileReferenceNumber = ROOT_NUMBER;
	ret->up.ParentFileReferenceNumber = 0;
	ret->us.v.FileNameLength = 0;
	ret->us.v.StrLen = 0;
	ret->us.v.dir = 1;
	ret->ut.v.suffixType = SF_DIR;
	ret->up.parent = NULL;
	ret->children = NULL;
	return ret;
}
static pFileEntry initDesktopFile(wchar_t *name, pFileEntry parent,BOOL dir){
	int str_len = (int)wcslen(name);
	int len = WCHAR_TO_UTF8_LEN(name,str_len);
	NEW0_FILE(ret,len);
	ret->us.v.FileNameLength = len;
	ret->us.v.StrLen = str_len;
	WCHAR_TO_UTF8(name,str_len,(LPSTR)ret->FileName,len);
	if(dir){
		ret->ut.v.suffixType = SF_DIR;
		ret->us.v.dir = 1;
	}else{
		ret->ut.v.suffixType = SF_LNK;
	}
	ret->us.v.system = 1;
	addChildren(parent,ret);
	//set_time(ret, &pfd->ftLastWriteTime);
	SET_SIZE(ret,0);
	ALL_FILE_COUNT +=1;
	return ret;
}


static void get_name(IShellFolder *f, LPITEMIDLIST pidlItems, SHGDNF flag, wchar_t *name){
	STRRET strDispName;
	f->GetDisplayNameOf(pidlItems, flag, &strDispName);
	StrRetToBuf(&strDispName, pidlItems, name, MAX_PATH);
}

static FileType get_type(IShellFolder *f, LPITEMIDLIST pidlItems){
	STRRET strDispName;
	char pszDisplayName[MAX_PATH];
	f->GetDisplayNameOf(pidlItems, SHGDN_FORPARSING, &strDispName);
	StrRetToBufA(&strDispName, pidlItems, pszDisplayName, MAX_PATH);
#ifdef MY_DEBUG
	printf("%s\n",pszDisplayName);
#endif
	if( strncmp(pszDisplayName+1,":\\",2)==0 ) return LOCAL;
	if( strncmp(pszDisplayName,"\\\\",2)==0 ) return NETWORK;
	if( strncmp(pszDisplayName,"::",2)==0 ) return VIRTUAL;
	return VIRTUAL;
}

static BOOL isFolder(IShellFolder *f, LPITEMIDLIST pidlItems){
    SFGAOF uAttr = SFGAO_FOLDER;
    f->GetAttributesOf(1, (LPCITEMIDLIST *) &pidlItems, &uAttr);
	return uAttr & SFGAO_FOLDER;
}

static void scan_desktop0(IShellFolder *f, LPITEMIDLIST pidlComplete, pFileEntry dir){
	HRESULT hr;
	LPENUMIDLIST ppenum = NULL;
	LPITEMIDLIST pidlItems = NULL;
	ULONG celtFetched;
    hr = f->EnumObjects(NULL,SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_INCLUDESUPERHIDDEN, &ppenum);
	if(hr!=S_OK) return;
    while( hr = ppenum->Next(1,&pidlItems, &celtFetched) == S_OK && (celtFetched) == 1){
		FileType ft = get_type(f,pidlItems);
		if(ft!=LOCAL){
			BOOL isDir = isFolder(f,pidlItems);
			wchar_t name[MAX_PATH];
			get_name(f,pidlItems,SHGDN_NORMAL,name);
			pFileEntry file = initDesktopFile(name,dir,isDir);
            if(isDir && ft==VIRTUAL){
				IShellFolder *psfFirstFolder = NULL;
                hr = f->BindToObject(pidlItems, NULL, IID_IShellFolder, (LPVOID *) &psfFirstFolder);
				LPITEMIDLIST	pidlComplete2 = ILCombine(pidlComplete,pidlItems);			
				scan_desktop0(psfFirstFolder,pidlComplete2,file);
				CoTaskMemFree(pidlComplete2);	
				psfFirstFolder->Release();
            }
        }
	}
	CoTaskMemFree(pidlItems);
	ppenum->Release();
}

static BOOL save_desktop_self(pFileEntry desktop){
	wchar_t fbuffer[MAX_PATH];
	DWORD size=MAX_PATH;
	if(GetUserName(fbuffer, &size)){
		save_desktop(fbuffer,desktop);
		return 1;
	}else{
		return 0;
	}
}

BOOL scan_desktop(){
	pFileEntry desktop=NULL;
    IShellFolder *root = NULL;
    CoInitialize( NULL );
	if(SHGetDesktopFolder(&root) != S_OK) return 0;
	desktop = genDesktopFileEntry();
	scan_desktop0(root, NULL, desktop);
    root->Release();
    CoUninitialize();
	save_desktop_self(desktop);
}

typedef google::sparse_hash_map<wchar_t *, pFileEntry> user_desktop;
user_desktop ud;

pFileEntry get_desktop(wchar_t *user_name){
	pFileEntry desktop = ud[user_name];
	if(desktop!=NULL) return desktop;
	load_desktop(user_name);
	return ud[user_name];
}

void put_desktop(wchar_t *user_name, pFileEntry desktop){
	ud[user_name] = desktop;
}



static BOOL exec_desktop0(IShellFolder *f, LPITEMIDLIST pidlComplete,wchar_t *str){
	BOOL ret=0;
	HRESULT hr;
	LPENUMIDLIST ppenum = NULL;
	LPITEMIDLIST pidlItems = NULL;
	ULONG celtFetched;
    hr = f->EnumObjects(NULL,SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_INCLUDESUPERHIDDEN, &ppenum);
	if(hr!=S_OK) return ret;
    while( hr = ppenum->Next(1,&pidlItems, &celtFetched) == S_OK && (celtFetched) == 1){
		wchar_t name[MAX_PATH];
		get_name(f,pidlItems,SHGDN_NORMAL,name);
		if(wcsncmp(name,str,wcslen(name))==0){
			if(wcslen(name)==wcslen(str)){
				open_pidl(pidlComplete,pidlItems);
				ret = 1;
			}else{
				wchar_t *nstr = str+wcslen(name);
				if(*nstr==L'\\'){
					IShellFolder *psfFirstFolder = NULL;
					hr = f->BindToObject(pidlItems, NULL, IID_IShellFolder, (LPVOID *) &psfFirstFolder);
					LPITEMIDLIST	pidlComplete2 = ILCombine(pidlComplete,pidlItems);			
					ret = exec_desktop0(psfFirstFolder,pidlComplete2,nstr+1);
					CoTaskMemFree(pidlComplete2);
					psfFirstFolder->Release();
				}
			}
			break;
		}
	}
	CoTaskMemFree(pidlItems);
	ppenum->Release();
	return ret;
}

BOOL exec_desktop(wchar_t *str){
	if(str==NULL || *str!=L'\\') return 0;
    IShellFolder *root = NULL;
	if(SHGetDesktopFolder(&root) != S_OK) return 0;
	BOOL ret = exec_desktop0(root, NULL, str+1);
    root->Release();
    CoUninitialize();
	return ret;
}

}// extern "C"