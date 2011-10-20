#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_ENV_H_
#define FILE_SEARCH_ENV_H_

#ifdef _MSC_VER
  #define INLINE __forceinline /* use __forceinline (VC++ specific) */
#else
  #define INLINE
#endif

	#include <stdio.h>	   
	#include <stdlib.h>
	#include <wchar.h>
		
#if defined(WIN32) 
	#if !defined(_WIN32_WINNT) || _WIN32_WINNT< 0x0501
	#define _WIN32_WINNT 0x0501
	#endif
	#pragma warning(disable:4996)
	#include <windows.h>
#else
	#define _DARWIN_C_SOURCE

	#include <sys/types.h>		/* some systems still require this */
	#include <sys/stat.h>
	#include <sys/termios.h>	/* for winsize */
	#ifndef TIOCGWINSZ
	#include <sys/ioctl.h>
	#endif
	#include <stddef.h>	   
	#include <string.h>	   
	#include <strings.h>   
	#include <unistd.h>	   
	#include <signal.h>		/* for SIG_ERR */
	#include <stdint.h>
	#include <time.h>

	
	#define strnicmp strncasecmp
	#define stricmp strcasecmp
	#define _wcsncoll wcsncmp
	#define _wcsnicoll wcsncasecmp
	
	#define	LISTENQ		1024	/* 2nd argument to listen() */
	#define	UNIXSTR_PATH	"/tmp/unix.str"	/* Unix domain stream cli-serv */
	#define	SA	struct sockaddr
	#define	FILE_MODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
					/* default file access permissions for new files */
	#define	DIR_MODE	(FILE_MODE | S_IXUSR | S_IXGRP | S_IXOTH)
					/* default permissions for new directories */				
	#define	MAXLINE	4096			/* max line length */
	#define BOOL int
	#define WCHAR wchar_t
	typedef int64_t LONGLONG;
	typedef uint64_t ULONGLONG;
	typedef int32_t DWORD;
	typedef char UINT;
	typedef void * HANDLE;
	typedef const wchar_t * LPCWSTR;
	typedef wchar_t * LPWSTR;	
	#define MAX_PATH 256
	#define IN
	#define OUT
	#define WINAPI

	typedef	void	Sigfunc(int);	/* for signal handlers */

	#if	defined(SIG_IGN) && !defined(SIG_ERR)
	#define	SIG_ERR	((Sigfunc *)-1)
	#endif

	#define	min(a,b)	((a) < (b) ? (a) : (b))
	#define	max(a,b)	((a) > (b) ? (a) : (b))
#endif




//#define USE_ZIP

#define MY_DEBUG



#include "GIGASOConfig.h"

#ifdef HAVE_64_BIT
	typedef unsigned long KEY; //NTFS文件的FileReferenceNumber类型，唯一标识一个文件。
#else
	typedef unsigned int KEY; //NTFS文件的FileReferenceNumber类型，唯一标识一个文件。
#endif

typedef unsigned char FILE_NAME_LEN; //文件名长度类型
typedef unsigned int MINUTE; //以分钟表示的时间类型
typedef unsigned short FSIZE; //文件大小类型
typedef unsigned char UTF8, *pUTF8; //UTF8类型的字符串


//排序方式常量定义
#define NO_ORDER 0
#define NAME_ORDER_ASC 1
#define NAME_ORDER_DESC 2
#define PATH_ORDER_ASC 3
#define PATH_ORDER_DESC 4
#define SIZE_ORDER_ASC 5
#define SIZE_ORDER_DESC 6
#define DATE_ORDER_ASC 7
#define DATE_ORDER_DESC 8

struct searchEnv{ //搜索的环境配置
	unsigned char order; //排序方式，缺省按名称排序
	BOOL case_sensitive; //是否大小写敏感，缺省不敏感
	unsigned char file_type;//指定搜索的类型
	BOOL offline; //查询offline文件还是online文件
	int path_len; //如果指定了搜索的路径，该路径名的字符长度
	WCHAR path_name[MAX_PATH]; ////指定搜索的路径
	WCHAR user_name[MAX_PATH]; ////执行搜索的当前用户名
};
typedef struct searchEnv SearchEnv, *pSearchEnv;

#define SearchWindowTitle L"豌豆搜索 -- 最好的文件搜索软件"
#define SearchWindowClass L"PeaSearch"
#define ListenerWindowClass L"PeaSearch_Listener"

#define ALL_DESKTOP L"desktop.all"


#define WM_SET_HOTKEY WM_USER+1
#define WM_GET_HOTKEY WM_USER+2

#define UPDATE_CHECH_FILE "gigaso_update"
#define UPDATE_CHECH_UNKNOWN '0'
#define UPDATE_CHECH_DONE '1'
#define UPDATE_CHECH_NEW '2'
#define UPDATE_CHECH_DOWNLOADING '3'

#endif  // FILE_SEARCH_ENV_H_

#ifdef __cplusplus
}
#endif
