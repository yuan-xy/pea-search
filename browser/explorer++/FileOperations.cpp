#include "stdafx.h"
#include <list>
#include "FileOperations.h"
#include "iDataObject.h"


using namespace std;

int RenameFile(TCHAR *NewFileName,TCHAR *OldFileName)
{
	SHFILEOPSTRUCT	shfo;

	shfo.hwnd	= NULL;
	shfo.wFunc	= FO_RENAME;
	shfo.pFrom	= OldFileName;
	shfo.pTo	= NewFileName;
	shfo.fFlags	= FOF_ALLOWUNDO|FOF_SILENT;

	return !SHFileOperation(&shfo);
}

BOOL PerformFileOperation(HWND Parent,TCHAR *Path,TCHAR *FileName,
TCHAR *Operation,TCHAR *Parameters)
{
	SHELLEXECUTEINFO	sei;
	HANDLE				hProcess = NULL;
	BOOL				bReturnValue;

	ZeroMemory(&sei,sizeof(SHELLEXECUTEINFO));
	sei.cbSize			= sizeof(SHELLEXECUTEINFO);
	sei.fMask			= SEE_MASK_FLAG_NO_UI|SEE_MASK_INVOKEIDLIST|SEE_MASK_NOCLOSEPROCESS;
	sei.hwnd			= Parent;
	sei.lpVerb			= Operation;
	sei.lpFile			= FileName;
	sei.lpParameters	= Parameters;
	sei.nShow			= SW_SHOW;
	sei.hProcess		= hProcess;

	bReturnValue = ShellExecuteEx(&sei);

	if(!bReturnValue)
	{
		TCHAR ExePath[MAX_PATH];

		/* File has failed to be opened/executed. If the file has no program
		associated with it, invoke the "Open With" system dialog box. */
		if((int)FindExecutable(FileName,NULL,ExePath) == SE_ERR_NOASSOC)
		{
			sei.lpVerb		= _T("openas");
			bReturnValue	= ShellExecuteEx(&sei);
		}
	}

	return bReturnValue;
}

BOOL ShowFileProperties(TCHAR *FileName)
{
	BOOL ReturnValue;

	ReturnValue = PerformFileOperation(NULL,NULL,FileName,_T("properties"),NULL);

	return ReturnValue;
}

int DeleteFiles(HWND hwnd,TCHAR *FileNames,BOOL Permanent)
{
	SHFILEOPSTRUCT	shfo;
	FILEOP_FLAGS	Flags = NULL;

	if(FileNames == NULL)
		return -1;

	if(!Permanent)
		Flags = FOF_ALLOWUNDO;

	shfo.hwnd					= hwnd;
	shfo.wFunc					= FO_DELETE;
	shfo.pFrom					= FileNames;
	shfo.pTo					= NULL;
	shfo.fFlags					= Flags;
	shfo.fAnyOperationsAborted	= NULL;
	shfo.hNameMappings			= NULL;
	shfo.lpszProgressTitle		= NULL;

	return SHFileOperation(&shfo);
}

int DeleteFilesToRecycleBin(HWND hwnd,TCHAR *FileNameList)
{
	return DeleteFiles(hwnd,FileNameList,FALSE);
}

int DeleteFilesPermanently(HWND hwnd,TCHAR *FileNameList)
{
	return DeleteFiles(hwnd,FileNameList,TRUE);
}



HRESULT CopyFiles(TCHAR *szFileNameList,int iListSize,IDataObject **pClipboardDataObject)
{
	return CopyFilesToClipboard(szFileNameList,iListSize,FALSE,pClipboardDataObject);
}

HRESULT CutFiles(TCHAR *szFileNameList,int iListSize,IDataObject **pClipboardDataObject)
{
	return CopyFilesToClipboard(szFileNameList,iListSize,TRUE,pClipboardDataObject);
}

HRESULT CopyFilesToClipboard(TCHAR *FileNameList,size_t iListSize,
BOOL bMove,IDataObject **pClipboardDataObject)
{
	DROPFILES	*df = NULL;
	FORMATETC	ftc[2];
	STGMEDIUM	stg[2];
	HGLOBAL		hglb = NULL;
	TCHAR		*ptr = NULL;
	DWORD		*pdwCopyEffect = NULL;
	size_t		iTempListSize = 0;
	HRESULT		hr;

	/* Manually calculate the list size if it is
	not supplied. */
	if(iListSize == -1)
	{
		ptr = FileNameList;

		while(*ptr != '\0')
		{
			iTempListSize += lstrlen(ptr) + 1;

			ptr += lstrlen(ptr) + 1;
		}

		iListSize = iTempListSize;

		iListSize++;
		iListSize *= sizeof(TCHAR);
	}

	ftc[0].cfFormat			= CF_HDROP;
	ftc[0].ptd				= NULL;
	ftc[0].dwAspect			= DVASPECT_CONTENT;
	ftc[0].lindex			= -1;
	ftc[0].tymed			= TYMED_HGLOBAL;

	ftc[1].cfFormat			= (CLIPFORMAT)RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
	ftc[1].ptd				= NULL;
	ftc[1].dwAspect			= DVASPECT_CONTENT;
	ftc[1].lindex			= -1;
	ftc[1].tymed			= TYMED_HGLOBAL;
	
	hglb = GlobalAlloc(GMEM_MOVEABLE,sizeof(DWORD));

	pdwCopyEffect = (DWORD *)GlobalLock(hglb);

	if(bMove)
		*pdwCopyEffect = DROPEFFECT_MOVE;
	else
		*pdwCopyEffect = DROPEFFECT_COPY;

	GlobalUnlock(hglb);

	stg[1].pUnkForRelease	= 0;

	stg[1].hGlobal			= hglb;
	stg[1].tymed			= TYMED_HGLOBAL;

	hglb = GlobalAlloc(GMEM_MOVEABLE,sizeof(DROPFILES) + iListSize);

	df = (DROPFILES *)GlobalLock(hglb);
	df->pFiles	= sizeof(DROPFILES);

	#ifdef UNICODE
	df->fWide = 1;
	#else
	df->fWide = 0;
	#endif

	LPBYTE pData = NULL;

	pData = (LPBYTE)df + sizeof(DROPFILES);

	memcpy(pData,FileNameList,iListSize);

	GlobalUnlock(hglb);

	stg[0].pUnkForRelease	= 0;
	stg[0].hGlobal			= hglb;
	stg[0].tymed			= TYMED_HGLOBAL;

	hr = CreateDataObject(ftc,stg,pClipboardDataObject,2);

	if(SUCCEEDED(hr))
	{
		hr = OleSetClipboard(*pClipboardDataObject);
	}

	return hr;
}

