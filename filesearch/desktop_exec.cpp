#include "env.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>

#include <shlobj.h>
#include <shlwapi.h>
#include <iostream>



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


static void get_name(IShellFolder *f, LPITEMIDLIST pidlItems, SHGDNF flag, wchar_t *name){
	STRRET strDispName;
	f->GetDisplayNameOf(pidlItems, flag, &strDispName);
	StrRetToBuf(&strDispName, pidlItems, name, MAX_PATH);
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

BOOL exec_desktop(const wchar_t *str){
	if(str==NULL || *str!=L'\\') return 0;
    IShellFolder *root = NULL;
	if(SHGetDesktopFolder(&root) != S_OK) return 0;
	BOOL ret = exec_desktop0(root, NULL, (wchar_t *) str+1);
    root->Release();
    CoUninitialize();
	return ret;
}

}// extern "C"