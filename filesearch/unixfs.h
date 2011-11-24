#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_MAC_H_
#define FILE_SEARCH_MAC_H_

#include "env.h"
#include "fs_common.h"

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


#endif  // FILE_SEARCH_MAC_H_

#ifdef __cplusplus
}
#endif
