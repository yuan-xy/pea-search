#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_SEARCH_H_
#define FILE_SEARCH_SEARCH_H_

#include "env.h"
#include "fs_common.h"


/**
* 根据查询要求搜索所有符合要求的文件
* @param str 查询条件字符串
* @param env 搜索的环境配置
* @param result 返回的所有文件的列表的地址
*/
extern DWORD search(WCHAR *str, pSearchEnv env,OUT pFileEntry **result);
extern void free_search(pFileEntry *p);

extern int * stat(WCHAR *str, pSearchEnv env);
extern int print_stat(int * stats, char *p);

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
