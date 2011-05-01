#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_SHARELIB_H_
#define FILE_SEARCH_SHARELIB_H_

#include "search.h"

struct searchRequest{
	int len;	/* Total length of request, not including this field */
	SearchEnv env;
	int from;
	int rows;
	WCHAR str[MAX_PATH];
};
typedef struct searchRequest SearchRequest, *pSearchRequest;

struct searchFile{
	MINUTE time;
	FSIZE size;
	FILE_NAME_LEN FileNameLength;
    UTF8  FileName[MAX_PATH]; //文件名，不包含\0，不是C语言风格的字符串，Unicode编码
	int pathLen;
    UTF8  path[1]; //文件名，不包含\0，不是C语言风格的字符串，Unicode编码
};
typedef struct searchFile SearchFile, *pSearchFile;

struct searchResponse{
	int len;	/* Total length of response, not including this field */
	SearchFile files[1];
};
typedef struct searchResponse SearchResponse, *pSearchResponse;

#define CS_TIMEOUT 5000

#define MAX_CLIENTS  5 /* Maximum number of clients for serverNP */

#define SERVER_PIPE L"\\\\.\\PIPE\\SERVER"


/* Commands for the statistics maintenance function. */

#define CS_INIT			1
#define CS_RQSTART		2
#define CS_RQCOMPLETE	3
#define CS_REPORT		4
#define CS_TERMTHD		5


#endif  // FILE_SEARCH_SHARELIB_H_

#ifdef __cplusplus
}
#endif
