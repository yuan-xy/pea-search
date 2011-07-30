#include "scheme_test.h"
#include "string_util.h"
#include "../PluginGigaso/bitmap.h"
#include <gdiplus.h>
#include <atlimage.h>

class ClientSchemeHandler : public CefThreadSafeBase<CefSchemeHandler>{
public:
  ClientSchemeHandler() : size_(0), offset_(0) {
	  bytes_ = (LPBYTE) malloc(4096);
  }

  ~ClientSchemeHandler() {
	  free(bytes_);
  }

  bool LoadBinaryResource(const WCHAR *file){
	SHFILEINFO shfi;
	HRESULT hr = SHGetFileInfo( file, 0, &shfi, sizeof( SHFILEINFO ), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS );
	if( !SUCCEEDED(hr) || shfi.hIcon==NULL) return false;
	HDC hdc = GetDC(NULL) ;
	ICONINFO info;
	GetIconInfo(shfi.hIcon,&info);
	HBITMAP  bmp = ReplaceColor(info.hbmColor,RGB(0,0,0), RGB(255,255,255), hdc);
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
	  std::wstring& mime_type, int* response_length)
  {
	  bool handled = false;
	  Lock();
	  std::wstring url = request->GetURL();
		url = UrlDecode(url);
	  if(LoadBinaryResource(url.c_str()+wcslen(L"gigaso://images/") )) {
		  handled = true;
		  mime_type = L"image/png";
	  }
	  *response_length = size_;
	  Unlock();
	  return handled;
  }

  virtual void Cancel(){}

  virtual bool ReadResponse(void* data_out, int bytes_to_read,
                            int* bytes_read)
  {
    bool has_data = false;
    *bytes_read = 0;
    Lock();
    if(offset_ < size_) {
      // Copy the next block of data into the buffer.
      int transfer_size = min(bytes_to_read, static_cast<int>(size_ - offset_));
      memcpy(data_out, bytes_ + offset_, transfer_size);
      offset_ += transfer_size;
      *bytes_read = transfer_size;
      has_data = true;
    }
    Unlock();
    return has_data;
  }

private:
  DWORD size_, offset_;
  LPBYTE bytes_;
  std::string html_;
};

// Implementation of the factory for for creating schema handlers.
class ClientSchemeHandlerFactory :
  public CefThreadSafeBase<CefSchemeHandlerFactory>
{
public:
  // Return a new scheme handler instance to handle the request.
  virtual CefRefPtr<CefSchemeHandler> Create()
  {
    return new ClientSchemeHandler();
  }
};

void InitSchemeTest()
{
  CefRegisterScheme(L"gigaso", L"images", new ClientSchemeHandlerFactory());
}

