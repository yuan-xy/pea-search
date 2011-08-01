#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_GLOBAL_H_
#define FILE_SEARCH_GLOBAL_H_

#include "env.h"
#include <windows.h>
#include "drive.h"
#include "ntfs.h"

#define DIRVE_COUNT 26
#define DIRVE_COUNT_OFFLINE DIRVE_COUNT*3

extern UTF8 rootNames[26][3];//根驱动器的名字

extern BOOL		g_bVols[];//对应驱动器是否存在
extern BOOL		g_loaded[];//对应驱动器的磁盘数据库是否已加载
extern BOOL		g_expires[];//对应驱动器的磁盘数据库是否已过期
extern DriveInfo	g_VolsInfo[]; //驱动器的信息
extern HANDLE      g_hVols[];//保存A~Z的对应的卷句柄

extern pFileEntry g_rootVols[];//保存根驱动器的FileEntry结构

extern USN         g_curFirstUSN[];
extern USN         g_curNextUSN[];
extern DWORDLONG   g_curJournalID[];
extern HANDLE      g_hThread[];//每个盘一个监视线程
extern DWORD       g_BytesPerCluster[];//每簇字节数
extern DWORD       g_FileRecSize[];//MFT文件记录大小，似乎还没有用到？
extern PBYTE       g_pOutBuffer[];//每个盘的文件块记录地址 初始时分配 结束时消

extern unsigned int ALL_FILE_COUNT; //当前所有文件的总数

extern unsigned char FILE_SEARCH_MAJOR_VERSION ;
extern unsigned char FILE_SEARCH_MINOR_VERSION ;











#endif  // FILE_SEARCH_GLOBAL_H_

#ifdef __cplusplus
}
#endif
