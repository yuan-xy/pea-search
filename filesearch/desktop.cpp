#include "env.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "suffix.h"
#include "search.h"
#include "global.h"
#include "util.h"
#include "fs_common.h"

#include <shlobj.h>
#include <shlwapi.h>
#include <iostream>

static int depth=0;
static pFileEntry desktop=NULL;

extern "C" {

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

static BOOL isVirtualFile(IShellFolder *f, LPITEMIDLIST pidlItems){
	STRRET strDispName;
	char pszDisplayName[MAX_PATH];
	f->GetDisplayNameOf(pidlItems, SHGDN_FORPARSING, &strDispName);
	StrRetToBufA(&strDispName, pidlItems, pszDisplayName, MAX_PATH);
	return strncmp(pszDisplayName+1,":\\",2)!=0;
}

static BOOL isFolder(IShellFolder *f, LPITEMIDLIST pidlItems){
    SFGAOF uAttr = SFGAO_FOLDER;
    f->GetAttributesOf(1, (LPCITEMIDLIST *) &pidlItems, &uAttr);
	return uAttr & SFGAO_FOLDER;
}

static void scan_desktop0(IShellFolder *f, pFileEntry desktop){
	HRESULT hr;
	LPENUMIDLIST ppenum = NULL;
	LPITEMIDLIST pidlItems = NULL;
	ULONG celtFetched;
    hr = f->EnumObjects(NULL,SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_INCLUDESUPERHIDDEN, &ppenum);
	if(hr!=S_OK) return;
    while( hr = ppenum->Next(1,&pidlItems, &celtFetched) == S_OK && (celtFetched) == 1){
		if(isVirtualFile(f,pidlItems)){
			BOOL isDir = isFolder(f,pidlItems);
			wchar_t name[MAX_PATH];
			get_name(f,pidlItems,SHGDN_NORMAL,name);
			pFileEntry file = initDesktopFile(name,desktop,isDir);
            if(isDir){
				IShellFolder *psfFirstFolder = NULL;
                hr = f->BindToObject(pidlItems, NULL, IID_IShellFolder, (LPVOID *) &psfFirstFolder);
				scan_desktop0(psfFirstFolder,file);
				psfFirstFolder->Release();
            }
        }
	}
	CoTaskMemFree(pidlItems);
	ppenum->Release();
}

void scan_desktop(){
	HRESULT hr;
    IShellFolder *psfDeskTop = NULL;
    CoInitialize( NULL );
    hr = SHGetDesktopFolder(&psfDeskTop);
	if(hr!=S_OK) return;
	desktop = genDesktopFileEntry();
	scan_desktop0(psfDeskTop, desktop);
    psfDeskTop->Release();
    CoUninitialize();
}

pFileEntry get_desktop(){
	return desktop;
}


}// extern "C"