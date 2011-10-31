#include <string_util.h>
#include <sstream>
#include <windows.h>


std::wstring StringToWString(const std::string& s)
{
	WCHAR* wch;
	UINT bytes = MultiByteToWideChar(CP_ACP, 0, s.c_str(), s.size()+1, NULL, 0);
	wch  = new WCHAR[bytes];
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

std::string StringReplace(const std::string& str, const std::string& from,
                          const std::string& to)
{
  std::string result = str;
  std::string::size_type pos = 0;
  std::string::size_type from_len = from.length();
  std::string::size_type to_len = to.length();
  do {
    pos = result.find(from, pos);
    if(pos != std::string::npos) {
      result.replace(pos, from_len, to);
      pos += to_len;
    }
  } while(pos != std::string::npos);
  return result;
}


static int decode1(const char c){
	if(c>='A') return c-'A'+10;
	return c-'0';
}

static int decode(const char *ss){
	return decode1(*ss)*16 + decode1(*(ss+1));
}

#define GBK_HZ_LEN 2

static std::string UrlDecode_Chinese(const std::string& str){
  std::string result = str;
  std::string::size_type pos = 0;
  do {
    pos = result.find("%", pos);
    if(pos != std::string::npos) {
	  std::string ch;
	  byte utf8[3];
	  WCHAR wc;
	  char c[GBK_HZ_LEN];
	  ch = result.substr(pos+1,2);
	  utf8[0] = decode(ch.c_str());
	  ch = result.substr(pos+4,2);
	  utf8[1] = decode(ch.c_str());
	  ch = result.substr(pos+7,2);
	  utf8[2] = decode(ch.c_str());
	  MultiByteToWideChar(CP_UTF8, 0, (char *)utf8, 3, &wc, 1);
	  WideCharToMultiByte(CP_ACP, 0, &wc, 1, c, GBK_HZ_LEN, NULL, NULL);
	  std::string chh(c, GBK_HZ_LEN);
      result.replace(pos, 9, chh);
      pos += GBK_HZ_LEN;
    }
  } while(pos != std::string::npos);
  return result;
}


std::string UrlDecode(const std::string& str){
	std::string url = str;
	  url = StringReplace(url, "/", "\\");
	  url = StringReplace(url, "%20", " ");
	  //url = StringReplace(url, "%23", "#");
	  //url = StringReplace(url, "%25", "%");
	  //url = StringReplace(url, "%7B", "{");
	  //url = StringReplace(url, "%7D", "}");
	  //url = StringReplace(url, "%5B", "[");
	  //url = StringReplace(url, "%5D", "]");
	  url = UrlDecode_Chinese(url);
  return url;
}
