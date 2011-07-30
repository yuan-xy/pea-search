#ifndef _CEFCLIENT_STRING_UTIL_H
#define _CEFCLIENT_STRING_UTIL_H

#include <string>


// Convert a std::string to a std::wstring
std::wstring StringToWString(const std::string& s);

// Convert a std::wstring to a std::string
std::string WStringToString(const std::wstring& s);

// Replace all instances of |from| with |to| in |str|.
std::wstring StringReplace(const std::wstring& str, const std::wstring& from,
                           const std::wstring& to);

std::string StringReplace(const std::string& str, const std::string& from,
                          const std::string& to);

std::string UrlDecode(const std::string& str);

#endif // _CEFCLIENT_STRING_UTIL_H
