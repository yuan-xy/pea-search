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

#include <windows.h>

typedef unsigned int KEY; //NTFS文件的FileReferenceNumber类型，唯一标识一个文件。
typedef unsigned char FILE_NAME_LEN; //文件名长度类型
typedef unsigned int MINUTE; //以分钟表示的时间类型
typedef unsigned short FSIZE; //文件大小类型
typedef unsigned char UTF8, *pUTF8; //UTF8类型的字符串




#endif  // FILE_SEARCH_ENV_H_

#ifdef __cplusplus
}
#endif
