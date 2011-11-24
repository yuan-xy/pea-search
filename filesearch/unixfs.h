#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_MAC_H_
#define FILE_SEARCH_MAC_H_

#include "env.h"
#include "fs_common.h"
#include <dirent.h>

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
     * @param dp 待访问的文件
     * @param data 环境数据
     */
    typedef void (*pDirentVisitor)(char *dir_name, DIR * dirp, struct dirent * dp, void *data);
    typedef BOOL (*pDirentVisitorB)(char *dir_name, DIR * dirp, struct dirent * dp, void *data);
    
    /**
     * 访问一个目录下的所有的文件，执行给定的访问函数
     * @param file 待遍历的目录
     * @param visitor 访问函数
     * @param data 环境数据
     */
    extern void dir_iterate(char *dir_name, pDirentVisitor visitor, void *data);
    extern BOOL dir_iterateB(char *dir_name, pDirentVisitorB visitor, void *data, char *buffer);
    
#endif  // FILE_SEARCH_MAC_H_

#ifdef __cplusplus
}
#endif
