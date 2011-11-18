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
extern int scanMac(pFileEntry root, int i);

/**
 * 启动MAC文件系统变动监视线程。该线程只能被启动一次。
 * @param i 驱动器编号
 */
extern BOOL StartMonitorThreadMAC(int i);

/**
 * 停止文件系统变动监视线程。
 * @param i 驱动器编号
 */
extern BOOL StopMonitorThreadMAC(int i);


#endif  // FILE_SEARCH_MAC_H_

#ifdef __cplusplus
}
#endif
