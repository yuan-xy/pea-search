#include <Shlobj.h>

#ifdef __cplusplus
extern "C" {
#endif
	
bool shortcut(LPWSTR lnkFile, LPTSTR destFile) {
	bool bReturn ;
	HRESULT hr;
	IShellLink *pShellLink;
	hr = CoInitialize(NULL);
	// if(hr!=S_OK) return false;
	hr = CoCreateInstance (CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,IID_IShellLink, (void **)&pShellLink);
	if(hr!=S_OK) return false;
	IPersistFile *ppf;
	hr = pShellLink-> QueryInterface(IID_IPersistFile, (void **)&ppf);
	if(hr!=S_OK) return false;
	hr = ppf-> Load(lnkFile, TRUE);
	if(hr!=S_OK) return false;
	hr = pShellLink-> GetPath(destFile, MAX_PATH, NULL, 0);
	if(hr!=S_OK) return false;
	ppf-> Release();
	pShellLink-> Release();
	CoUninitialize(); 
	return true;
}


#ifdef __cplusplus
}
#endif