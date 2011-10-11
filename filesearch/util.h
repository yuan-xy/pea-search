#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_UTIL_H_
#define FILE_SEARCH_UTIL_H_

#include "env.h"
#include <string.h>
#include <stdio.h>

/**
 * 报告windows系统调用错误
 */
#define WIN_ERROR fprintf(stderr,"error code : %d , line %d in '%s'\n",GetLastError(), __LINE__, __FILE__);
#define MEM_ERROR fprintf(stderr,"insufficient memory, line %d in '%s'\n", __LINE__, __FILE__);
#define CPP_ERROR fprintf(stderr,"c++ exception, line %d in '%s'\n", __LINE__, __FILE__);

/**
 * 仅在debug模式下调用assert
 * @param exp 必须为真的表达式
 */
extern void assert_debug(int exp);

/**
 * 进程不退出的自定义assert
 * @param exp 必须为真的表达式
 * @param ret 出错返回值
 */
#define my_assert(exp,ret) \
	if(!(exp)){\
		fprintf(stderr,"error assert, line %d in '%s'\n",  __LINE__, __FILE__);\
		assert_debug(exp);\
		return ret;\
	}


/**
* 测量函数f（无参无返回值）执行所需的毫秒数
**/
extern int time_passed(void (*f)());
/**
* 测量函数f（有一个参数）执行所需的毫秒数
**/
extern int time_passed_p1(void (*f)(void *),void *);
/**
* 测量函数f（有两个个参数）执行所需的毫秒数
**/
extern int time_passed_p2(void (*f)(void *,void *),void *,void *);
/**
* 测量函数f（有返回值）执行所需的毫秒数
**/
extern int time_passed_ret(int (*f)(),int *ret_data);

/**
 * 将unicode编码的字符串in转换为utf-8编码的字符串
 * @param insize_c表示The number of Unicode (16-bit) characters in the string
 * @return 动态分配的UTF8串，以及out_size_b表示以byte统计的字符串长度
 * 
 */
extern pUTF8 wchar_to_utf8(const WCHAR *in, int insize_c, int *out_size_b);
extern int wchar_to_utf8_len(const WCHAR *in, int insize_c);
extern void wchar_to_utf8_nocheck(const WCHAR *in, int insize_c, pUTF8 out, int out_size);

extern WCHAR* utf8_to_wchar(const pUTF8 in, int insize_b, int *out_size_c);
extern int utf8_to_wchar_len(const pUTF8 in, int insize_b);
extern int utf8_to_wchar_nocheck(const pUTF8 in, int insize_b, wchar_t *out, int out_buffer_size);

extern wchar_t *wcsrchr_me(const wchar_t *S, int len, const wchar_t C);

//定义二进制常量
#define BYTE_BIN(n) ( \
((0##n%0100000000/010000000>0)<<7)| \
((0##n%010000000 /01000000 >0)<<6)| \
((0##n%01000000 /0100000 >0)<<5)| \
((0##n%0100000   /010000   >0)<<4)| \
((0##n%010000    /01000    >0)<<3)| \
((0##n%01000     /0100     >0)<<2)| \
((0##n%0100      /010      >0)<<1)| \
((0##n%010       /01       >0)<<0))

#define WORD_BIN(n) ( \
(BYTE_BIN(0##n/0100000000)<<8)|BYTE_BIN(0##n%0100000000) \
)

#define DWORD_BIN(highword,lowword) ( \
(WORD_BIN(0##highword)<<16)|WORD_BIN(0##lowword) \
)


/**
 * 定义一个变量并在堆上分配空间
 * type 变量类型
 * var  变量指针名
 */
#define NEW(type,var) type *var = (type *)malloc_safe(sizeof(type))
/**
 * 定义一个变量并在堆上分配空间，同时将该内存区域清零。
 * type 变量类型
 * var  变量指针名
 */
#define NEW0(type,var) NEW(type,var); memset(var,0,sizeof(type));

/**
 * 在堆上分配大小为len的空间
 * @param len 以byte计数的内存空间大小
 */
extern void * malloc_safe(size_t len);
extern void * realloc_safe(void *ptr, size_t len);
/**
 * 释放由malloc_safe分配的空间
 * @param ptr 指向待释放的内存的指针
 */
extern void free_safe(void *ptr);

/**
 * 将从系统调用中获得的文件大小转换为内部定义的大小
 */
extern FSIZE file_size_shorten(ULONGLONG size);


/**
 * 文件FSIZE的度量单位（byte\KB\MB\GB）,用两位二进制数表示
 */
extern int file_size_unit(FSIZE size);

/**
 * 文件FSIZE的大小，0-1000之间
 */
extern int file_size_amount(FSIZE size);

#define MD5_LEN 16

extern void MD5Str(char *string, char *md5);

extern void MD5File(char *filename, char *md5);



#endif  // FILE_SEARCH_UTIL_H_

#ifdef __cplusplus
}
#endif
