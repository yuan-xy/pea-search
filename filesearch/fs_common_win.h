#ifdef WIN32

#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_FS_COMMON_WIN_H_
#define FILE_SEARCH_FS_COMMON_WIN_H_

#include "env.h"
#include "common.h"

/*
 * 得到该文件所属驱动盘编号
 */
extern int getDrive(pFileEntry file);


#define is_readonly(x) x->FileAttributes&FILE_ATTRIBUTE_READONLY
#define is_hidden(x)  x->FileAttributes&FILE_ATTRIBUTE_HIDDEN
#define is_system(x)  x->FileAttributes&FILE_ATTRIBUTE_SYSTEM
#define is_dir(x)     x->FileAttributes&FILE_ATTRIBUTE_DIRECTORY

#define is_readonly_ffd(find_data) find_data->dwFileAttributes&FILE_ATTRIBUTE_READONLY
#define is_hidden_ffd(find_data)  find_data->dwFileAttributes&FILE_ATTRIBUTE_HIDDEN
#define is_system_ffd(find_data)  find_data->dwFileAttributes&FILE_ATTRIBUTE_SYSTEM
#define is_dir_ffd(find_data)     find_data->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY

extern void FileRemoveFilter(pFileEntry file, void *data);

extern BOOL CloseVolumeHandle(int i);

#endif  // FILE_SEARCH_FS_COMMON_WIN_H_

#ifdef __cplusplus
}
#endif


#endif //WIN32