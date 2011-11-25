#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_MAC_H_
#define FILE_SEARCH_MAC_H_

#include "env.h"
#include "fs_common.h"
#include <dirent.h>
#include <stdarg.h>

/**
 * 扫描一个根分区
 * @param root 根分区
 */
extern int scanUnix(pFileEntry root, int i);

    /**
     * 给定的文件是否应该被忽略. 不索引该目录下的所有文件，不需要查询
     * @param fullpath 文件全路径
     * @param filename 文件名     
     */    
    extern BOOL ignore_dir(char *fullpath, char *dirname);
    extern BOOL ignore_dir2(char *fullpath);

    extern pFileEntry initUnixFile(const struct stat *statptr, char *filename, pFileEntry parent);

    extern BOOL same_file(pFileEntry file, struct dirent * dp);

    /**
     * 在迭代中访问一个文件
     * @param dp 迭代的目录名
     * @param dirp 目录的DIR *指针
     * @param dp 待访问的文件
     * @param args 可变参数
     * @return 是否提前结束循环
     */
    typedef BOOL (*pDirentVisitorB)(char *dir_name, DIR * dirp, struct dirent * dp, va_list args);
    typedef void (*pDirentVisitor)(char *dir_name, DIR * dirp, struct dirent * dp, va_list args);
    
    /**
     * 访问一个目录下的所有的文件，执行给定的访问函数
     * @param visitor 访问函数
     * @param dir_name 待遍历的目录名
     * @param ... 可变参数
     */
    extern void dir_iterate(pDirentVisitor visitor, char *dir_name, ...);
    
    /**
     * 访问一个目录下的所有的文件，执行给定的访问函数。如果pDirentVisitorB返回1, 提前结束循环。
     * @param visitor 访问函数
     * @param dir_name 待遍历的目录名
     * @param buffer 如果提前结束循环，此时的文件名
     * @param ... 可变参数
     * @return 循环是否提前结束
     */
    extern BOOL dir_iterateB(pDirentVisitorB visitor, char *dir_name, char *buffer, ...);
    
#endif  // FILE_SEARCH_MAC_H_

#ifdef __cplusplus
}
#endif
