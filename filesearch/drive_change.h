#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_DRIVE_CHANGE_H_
#define FILE_SEARCH_DRIVE_CHANGE_H_

#include "env.h"
#include <windows.h>

/**
 * 可插拔磁盘变动监听器
 * @param drive 磁盘序号
 * @param isadd 是插入磁盘还是拔出磁盘
 */
typedef void (*pDriveChangeListener)(int drive, BOOL isadd);


/**
 * 启动可插拔磁盘变动监视线程。该线程只能被启动一次。
 * @param pDriveChangeListener 变动监听器
 */
extern BOOL StartDriveChangeMonitorThread( pDriveChangeListener );

/**
 * 停止可插拔磁盘变动监视线程
 */
extern BOOL StopDriveChangeMonitorThread(void);


#endif  // FILE_SEARCH_DRIVE_CHANGE_H_

#ifdef __cplusplus
}
#endif