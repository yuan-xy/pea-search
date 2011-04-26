#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_FAT_H_
#define FILE_SEARCH_FAT_H_

#include "env.h"
#include "fs_common.h"

/**
 * 扫描一个根分区
 * @param root 根分区
 */
extern int scanRoot(pFileEntry root, int i);

/**
 * 打开一个驱动器句柄以便监视文件变动
 * @param i 驱动器编号
 */
extern void OpenFatHandle(int i);

/**
 * 启动FAT文件系统变动监视线程。该线程只能被启动一次。
 * @param i 驱动器编号
 */
extern BOOL StartMonitorThreadFAT(int i);

#endif  // FILE_SEARCH_FAT_H_

#ifdef __cplusplus
}
#endif
