#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_ENV_H_
#define FILE_SEARCH_ENV_H_

#if !defined(_WIN32_WINNT) || _WIN32_WINNT< 0x0500
#define _WIN32_WINNT 0x0500
#endif

#define UNICODE
#define _UNICODE

#pragma warning(disable:4996)

//#define USE_ZIP

#define MY_DEBUG

#define WORD_SIZE 4


#define SHARELIB_API __declspec(dllexport)
#include "sharelib.h"

#endif  // FILE_SEARCH_ENV_H_

#ifdef __cplusplus
}
#endif
