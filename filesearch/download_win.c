#include "env.h"
#include <Urlmon.h>
#include <Wininet.h>
#include "download.h"

BOOL download(const WCHAR *url, const WCHAR *filename){
	HRESULT hr;
	DeleteUrlCacheEntry(url);
	hr = URLDownloadToFile(NULL,url,filename,0,NULL);
	return (hr==S_OK);
}
