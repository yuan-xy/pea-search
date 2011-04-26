#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_SHARELIB_H_
#define FILE_SEARCH_SHARELIB_H_

#include <windows.h>

#ifndef SHARELIB_API
#define SHARELIB_API __declspec(dllimport)
#endif

typedef unsigned int KEY; //NTFS文件的FileReferenceNumber类型，唯一标识一个文件。

typedef unsigned char FILE_NAME_LEN; //文件名长度类型

typedef unsigned int MINUTE; //以分钟表示的时间类型

typedef unsigned short FSIZE; //文件大小类型

typedef unsigned char UTF8, *pUTF8; //UTF8类型的字符串

struct fileEntry{  //表示一个文件
	KEY FileReferenceNumber;  //NTFS的当前文件FileReferenceNumber
	union {
		KEY ParentFileReferenceNumber;  //NTFS的父目录文件FileReferenceNumber
		struct fileEntry *parent; //父目录
	} up;
	void *children; //子文件列表, pointer to std::vector<pFileEntry>
	union {
		unsigned int value;
		struct {
			unsigned char readonly:1; //是否只读
			unsigned char hidden:1; //是否隐藏
			unsigned char system:1; //是否系统文件
			unsigned char dir:1;    //是否是目录
			unsigned char size1:4;  //size的高4位
			unsigned char size2:8;  //size的低8位
			FILE_NAME_LEN FileNameLength:8; //文件名字节长度
			FILE_NAME_LEN StrLen:8; //文件名的字符长度
		} v;
	} us;
	union {
		unsigned int value;
		struct {
			unsigned char time1; //时间高字节
			unsigned char time2; //时间中字节
			unsigned char time3; //时间低字节
			unsigned char suffixType; //文件后缀名所属类型
		} v;
	} ut;
    UTF8  FileName[2]; //文件名，不包含\0，不是C语言风格的字符串，Unicode编码
};
typedef struct fileEntry FileEntry, *pFileEntry;

#define IsReadonly(pFileEntry) (pFileEntry->us.v.readonly==1)
#define IsHidden(pFileEntry) (pFileEntry->us.v.hidden==1)
#define IsSystem(pFileEntry) (pFileEntry->us.v.system==1)
#define IsDir(pFileEntry) (pFileEntry->us.v.dir==1)



SHARELIB_API extern MINUTE GET_TIME(pFileEntry file);
SHARELIB_API extern FSIZE GET_SIZE(pFileEntry file);

/**
 * 将从系统调用中获得的文件大小转换为内部定义的大小
 */
SHARELIB_API extern FSIZE file_size_shorten(ULONGLONG size);

/**
 * 文件FSIZE的度量单位（byte\KB\MB\GB）,用两位二进制数表示
 */
SHARELIB_API extern int file_size_unit(FSIZE size);

/**
 * 文件FSIZE的大小，0-1000之间
 */
SHARELIB_API extern int file_size_amount(FSIZE size);

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
	int path_len; //如果指定了搜索的路径，该路径名得byte长度
	WCHAR *path_name; ////指定搜索的路径
};
typedef struct searchEnv SearchEnv, *pSearchEnv;

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
SHARELIB_API extern void * malloc_safe(size_t len);
SHARELIB_API extern void * realloc_safe(void *ptr, size_t len);
/**
 * 释放由malloc_safe分配的空间
 * @param ptr 指向待释放的内存的指针
 */
SHARELIB_API extern void free_safe(void *ptr);

/**
* 根据查询要求搜索所有符合要求的文件
* @param str 查询条件字符串
* @param env 搜索的环境配置
* @param result 返回的所有文件的列表的地址
*/
SHARELIB_API extern DWORD search(WCHAR *str, pSearchEnv env,OUT pFileEntry **result);
/**
* 根据文件名查找到对应的pFileEntry。非精确路径匹配，支持部分模糊查询。
* @param name 待查找文件的全路径名
* @len 路径名的byte长度
*/
SHARELIB_API extern pFileEntry find_file(WCHAR *name, int len);

SHARELIB_API extern void print_full_path(pFileEntry file);

SHARELIB_API extern BOOL gigaso_init();
SHARELIB_API extern BOOL gigaso_destory();

#endif  // FILE_SEARCH_SHARELIB_H_

#ifdef __cplusplus
}
#endif
