#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_Linux_H_
#define FILE_SEARCH_Linux_H_

#include "env.h"
#include "fs_common.h"

/**
 * 启动Linux文件系统变动监视线程。该线程只能被启动一次。
 * @param i 驱动器编号
 */
extern BOOL StartMonitorThreadLinux(int i);

/**
 * 停止文件系统变动监视线程。
 * @param i 驱动器编号
 */
extern BOOL StopMonitorThreadLinux(int i);


#endif  // FILE_SEARCH_Linux_H_

#ifdef __cplusplus
}
#endif
