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
static pFileEntry desktop=NULL;

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

static void scan_desktop0(IShellFolder *f, pFileEntry dir, LPITEMIDLIST pidlComplete){
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
			open_pidl(pidlComplete,pidlItems);
			pFileEntry file = initDesktopFile(name,dir,isDir);
            if(isDir && ft==VIRTUAL){
				IShellFolder *psfFirstFolder = NULL;
                hr = f->BindToObject(pidlItems, NULL, IID_IShellFolder, (LPVOID *) &psfFirstFolder);
				LPITEMIDLIST	pidlComplete2 = ILCombine(pidlComplete,pidlItems);			
				scan_desktop0(psfFirstFolder,file,pidlComplete2);
				CoTaskMemFree(pidlComplete2);	
				psfFirstFolder->Release();
            }
        }
	}
	CoTaskMemFree(pidlItems);
	ppenum->Release();
}

/*
如何根据parseing name执行该文件？如何根据pidl打开它呢？SHELLEXECUTEINFO

控制面板的parseing name为：
::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\::{21EC2020-3AEA-1069-A2DD-08002B30309D}
可以直接运行，或者运行explorer ::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\::{21EC2020-3AEA-1069-A2DD-08002B30309D}

另外一种运行方式为：
rundll32.exe shell32.dll,Control_RunDLL

添加或删除程序的parseing name为自身，这是如何执行它？
如下命令可以运行: rundll32.exe shell32.dll,Control_RunDLL appwiz.cpl
但是如何知道appwiz.cpl就是添加或删除程序。



*/
static void parse_name(){
	HRESULT hr;
    IShellFolder *psfDeskTop = NULL;
	LPITEMIDLIST pidl;
	ULONG cbEaten;
    CoInitialize( NULL );
    hr = SHGetDesktopFolder(&psfDeskTop);
	hr = psfDeskTop->ParseDisplayName(NULL,NULL,L"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::{21EC2020-3AEA-1069-A2DD-08002B30309D}",&cbEaten, &pidl,NULL);
    psfDeskTop->Release();
    CoUninitialize();
}

BOOL scan_desktop(){
	HRESULT hr;
    IShellFolder *psfDeskTop = NULL;
    CoInitialize( NULL );
    hr = SHGetDesktopFolder(&psfDeskTop);
	if(hr!=S_OK) return 0;
	desktop = genDesktopFileEntry();
	scan_desktop0(psfDeskTop, desktop, NULL);
    psfDeskTop->Release();
    CoUninitialize();
	{
		wchar_t fbuffer[MAX_PATH];
		DWORD size=MAX_PATH;
		if(GetUserName(fbuffer, &size)){
			save_desktop(fbuffer,desktop);
			return 1;
		}else{
			return 0;
		}
	}
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


}// extern "C"