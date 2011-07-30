#include <string_util.h>
#include <sstream>
#include <windows.h>


std::wstring StringToWString(const std::string& s)
{
	wchar_t* wch;
	UINT bytes = MultiByteToWideChar(CP_ACP, 0, s.c_str(), s.size()+1, NULL, 0);
	wch  = new wchar_t[bytes];
	if(wch)
		bytes = MultiByteToWideChar(CP_ACP, 0, s.c_str(), s.size()+1, wch, bytes);
  std::wstring str = wch;
	delete [] wch;
  return str;
}

std::string WStringToString(const std::wstring& s)
{
	char* ch;
	UINT bytes = WideCharToMultiByte(CP_ACP, 0, s.c_str(), s.size()+1, NULL, 0,
                                   NULL, NULL); 
	ch = new char[bytes];
	if(ch)
		bytes = WideCharToMultiByte(CP_ACP, 0, s.c_str(), s.size()+1, ch, bytes,
                                NULL, NULL);
	std::string str = ch;
	delete [] ch;
  return str;
}


std::wstring StringReplace(const std::wstring& str, const std::wstring& from,
                           const std::wstring& to)
{
  std::wstring result = str;
  std::wstring::size_type pos = 0;
  std::wstring::size_type from_len = from.length();
  std::wstring::size_type to_len = to.length();
  do {
    pos = result.find(from, pos);
    if(pos != std::wstring::npos) {
      result.replace(pos, from_len, to);
      pos += to_len;
    }
  } while(pos != std::wstring::npos);
  return result;
}

static int decode1(const wchar_t c){
	if(c>=L'A') return c-L'A'+10;
	return c-L'0';
}

static int decode(const wchar_t *ss){
	return decode1(*ss)*16 + decode1(*(ss+1));
}

static std::wstring UrlDecode_Chinese(const std::wstring& str){
  std::wstring result = str;
  std::wstring::size_type pos = 0;
  do {
    pos = result.find(L"%", pos);
    if(pos != std::wstring::npos) {
	  std::wstring ch;
	  byte utf8[3];
	  wchar_t c;
	  ch = result.substr(pos+1,2);
	  utf8[0] = decode(ch.c_str());
	  ch = result.substr(pos+4,2);
	  utf8[1] = decode(ch.c_str());
	  ch = result.substr(pos+7,2);
	  utf8[2] = decode(ch.c_str());
	  MultiByteToWideChar(CP_UTF8, 0, (char *)utf8, 3, &c, 1);
	  std::wstring chh(1, c);
      result.replace(pos, 9, chh);
      pos += 1;
    }
  } while(pos != std::wstring::npos);
  return result;
}


std::wstring UrlDecode(const std::wstring& str){
	std::wstring url = str;
	  url = StringReplace(url, L"/", L"\\");
	  url = StringReplace(url, L"%20", L" ");
	  //url = StringReplace(url, L"%23", L"#");
	  //url = StringReplace(url, L"%25", L"%");
	  //url = StringReplace(url, L"%7B", L"{");
	  //url = StringReplace(url, L"%7D", L"}");
	  //url = StringReplace(url, L"%5B", L"[");
	  //url = StringReplace(url, L"%5D", L"]");
	  url = UrlDecode_Chinese(url);
  return url;
}
