#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_WRITE_H_
#define FILE_SEARCH_WRITE_H_

#include "fs_common.h"

/**
 * 将给定驱动器的所有文件信息写入硬盘
 * @param i 驱动器编号
 */
extern BOOL save_db(int i);

/**
 * 从硬盘中读取给定驱动器的文件数据库
 * @param i 驱动器编号
 */
extern BOOL load_db(int i);


/**
 * 加载所有的离线的db文件
 */
extern int load_offline_dbs();

extern void del_offline_db(int i);

/**
 * 访问给定的db文件
 * @param db_name  db文件名
 * @return 是否继续迭代
 */
typedef BOOL (*pDbVisitor)(char *db_name, void *data);

/**
 * 遍历所有的db文件。
 */
extern void DbIterator(pDbVisitor, void *data);

extern BOOL save_desktop(wchar_t *user_name, pFileEntry desktop);

extern BOOL load_desktop(wchar_t *user_name);

#endif  // FILE_SEARCH_WRITE_H_

#ifdef __cplusplus
}
#endif