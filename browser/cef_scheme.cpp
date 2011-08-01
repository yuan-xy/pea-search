#include "../3rd/cef_binary/include/cef_wrapper.h"
#include "cef_scheme.h"
#include "string_util.h"
#include "cef_util.h"
#include "../PluginGigaso/bitmap.h"
#include <gdiplus.h>
#include <atlimage.h>

extern HWND hMainWin;

class ClientSchemeHandler : public CefSchemeHandler
{
public:
  ClientSchemeHandler() : offset_(0) {bytes_ = (LPBYTE) malloc(4096);}
  ~ClientSchemeHandler() {free(bytes_);}

  bool LoadBinaryResource(const WCHAR *file){
	SHFILEINFO shfi;
	HRESULT hr = SHGetFileInfo( file, 0, &shfi, sizeof( SHFILEINFO ), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS );
	if( !SUCCEEDED(hr) || shfi.hIcon==NULL) return false;
	ICONINFO info;
	GetIconInfo(shfi.hIcon,&info);
/*
	HDC hdc = GetDC(hMainWin) ;
	HDC hdcMem = CreateCompatibleDC(hdc);
	SelectObject (hdcMem, info.hbmColor) ;
	HDC hdcMask = CreateCompatibleDC(hdc);
	SelectObject (hdcMask, info.hbmMask) ;
    BitBlt (hdcMem, 0, 0, 16, 16, 
                  hdcMask, 0, 0, SRCPAINT) ;
	ReleaseDC(hMainWin,hdcMem);
	ReleaseDC(hMainWin,hdc);
*/
	HDC hdc = GetDC(NULL) ;
	HBITMAP  bmp = ReplaceColor(info.hbmColor,RGB(0,0,0), RGB(255,255,255), hdc);
	ReleaseDC(NULL,hdc);
	CImage image;
	image.Attach(bmp);
	IStream* stream;
	hr = CreateStreamOnHGlobal(0, TRUE, &stream);
	if( !SUCCEEDED(hr) ) return false;
	image.Save(stream,Gdiplus::ImageFormatPNG);
	DestroyIcon(shfi.hIcon);
    ULARGE_INTEGER liSize;
    IStream_Size(stream, &liSize);
	size_ = liSize.LowPart;
	IStream_Reset(stream);
	IStream_Read(stream, bytes_, size_);
	stream->Release();
	return true;
  }

  virtual bool ProcessRequest(CefRefPtr<CefRequest> request,
                              CefString& redirectUrl,
                              CefRefPtr<CefResponse> response,
                              int* response_length)
  {
	  REQUIRE_IO_THREAD();
	  bool handled = false;
	  AutoLock lock_scope(this);
	  try{
		  std::string url = request->GetURL();
		  url = UrlDecode(url);
		  std::wstring urlw = StringToWString(url);
		  if(LoadBinaryResource(urlw.c_str()+wcslen(L"gigaso://images/") )) {
			  handled = true;
			  // Set the resulting mime type
			  response->SetMimeType("image/png");
			  response->SetStatus(200);
		  }
		  *response_length = size_;
	  }catch(...){}
	  return handled;
  }

  // Cancel processing of the request.
  virtual void Cancel()
  {
    REQUIRE_IO_THREAD();
  }

  // Copy up to |bytes_to_read| bytes into |data_out|. If the copy succeeds
  // set |bytes_read| to the number of bytes copied and return true. If the
  // copy fails return false and ReadResponse() will not be called again.
  virtual bool ReadResponse(void* data_out, int bytes_to_read,
                            int* bytes_read)
  {
    REQUIRE_IO_THREAD();

    bool has_data = false;
    *bytes_read = 0;

    AutoLock lock_scope(this);

    if(offset_ < size_) {
      // Copy the next block of data into the buffer.
      int transfer_size =
          min(bytes_to_read, static_cast<int>(size_ - offset_));
      memcpy(data_out, bytes_ + offset_, transfer_size);
      offset_ += transfer_size;

      *bytes_read = transfer_size;
      has_data = true;
    }

    return has_data;
  }

private:
  DWORD size_, offset_;
  LPBYTE bytes_;

  IMPLEMENT_REFCOUNTING(ClientSchemeHandler);
  IMPLEMENT_LOCKING(ClientSchemeHandler);
};

// Implementation of the factory for for creating schema handlers.
class ClientSchemeHandlerFactory : public CefSchemeHandlerFactory
{
public:
  // Return a new scheme handler instance to handle the request.
  virtual CefRefPtr<CefSchemeHandler> Create(const CefString& scheme_name,
                                             CefRefPtr<CefRequest> request)
  {
    REQUIRE_IO_THREAD();
    return new ClientSchemeHandler();
  }

  IMPLEMENT_REFCOUNTING(ClientSchemeHandlerFactory);
};

void InitSchemeTest()
{
  CefRegisterCustomScheme("gigaso", true, false, false);
  CefRegisterSchemeHandlerFactory("gigaso", "images",
      new ClientSchemeHandlerFactory());
}
