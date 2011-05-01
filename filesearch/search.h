#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_SEARCH_H_
#define FILE_SEARCH_SEARCH_H_

#include "env.h"
#include "fs_common.h"

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
	WCHAR path_name[MAX_PATH]; ////指定搜索的路径
};
typedef struct searchEnv SearchEnv, *pSearchEnv;


/**
* 根据查询要求搜索所有符合要求的文件
* @param str 查询条件字符串
* @param env 搜索的环境配置
* @param result 返回的所有文件的列表的地址
*/
extern DWORD search(WCHAR *str, pSearchEnv env,OUT pFileEntry **result);
/**
* 根据文件名查找到对应的pFileEntry。非精确路径匹配，支持部分模糊查询。
* @param name 待查找文件的全路径名
* @len 路径名的byte长度
*/
extern pFileEntry find_file(WCHAR *name, int len);

#endif  // FILE_SEARCH_SEARCH_H_

#ifdef __cplusplus
}
#endif
