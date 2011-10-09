#include "env.h"
#include <Urlmon.h>
#include <Wininet.h>
#include "download.h"

BOOL download(const wchar_t *url, const wchar_t *filename){
	HRESULT hr;
	DeleteUrlCacheEntry(url);
	hr = URLDownloadToFile(NULL,url,filename,0,NULL);
	return (hr==S_OK);
}
