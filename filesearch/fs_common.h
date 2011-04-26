#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_FS_COMMON_H_
#define FILE_SEARCH_FS_COMMON_H_

#include "env.h"
#include <windows.h>

#define FILE_ENTRY_SIZE_(name_len_bytes) (sizeof(FileEntry)+(name_len_bytes)-sizeof(int))
#define FILE_ENTRY_SIZE(file) FILE_ENTRY_SIZE_(file->us.v.FileNameLength)


#define NEW0_FILE(file,name_len_bytes) \
		pFileEntry file = (pFileEntry) malloc_safe(FILE_ENTRY_SIZE_(name_len_bytes)); \
		memset(file,0,FILE_ENTRY_SIZE_(name_len_bytes));

#define SET_ROOT_NAME(root,name) memcpy_s(root->FileName,2,name,2)

#define FERROR(file)  fprintf(stderr,"error: %s , line %d in '%s'\n",((file==NULL || file->FileName ==NULL)? "null":file->FileName), __LINE__, __FILE__);

/*
 * 得到该文件所属驱动盘编号
 */
extern int getDrive(pFileEntry file);
/*
 * 以MultiByte编码打印该文件的文件名
 */
extern void PrintFilenameMB(pFileEntry file);
/*
 * 打印该文件的全路径
 */

/*
 * 打印该文件的日期
 */
extern void print_time(pFileEntry file);

#define is_readonly(x) x->FileAttributes&FILE_ATTRIBUTE_READONLY
#define is_hidden(x)  x->FileAttributes&FILE_ATTRIBUTE_HIDDEN
#define is_system(x)  x->FileAttributes&FILE_ATTRIBUTE_SYSTEM
#define is_dir(x)     x->FileAttributes&FILE_ATTRIBUTE_DIRECTORY

#define is_readonly_ffd(find_data) find_data->dwFileAttributes&FILE_ATTRIBUTE_READONLY
#define is_hidden_ffd(find_data)  find_data->dwFileAttributes&FILE_ATTRIBUTE_HIDDEN
#define is_system_ffd(find_data)  find_data->dwFileAttributes&FILE_ATTRIBUTE_SYSTEM
#define is_dir_ffd(find_data)     find_data->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY

extern MINUTE ConvertSystemTimeToMinute(SYSTEMTIME sysTime);

extern void ConvertMinuteToSystemTime(SYSTEMTIME *sysTime,IN MINUTE time32);

extern void SET_TIME(pFileEntry file, MINUTE time);

extern void SET_SIZE(pFileEntry file, FSIZE size);

/**
 * 将从文件系统中获得的时间转换为MINUTE赋值给文件
 */
extern void set_time(pFileEntry file, PFILETIME time);

/**
 * 生成驱动器根文件的pFileEntry结构
 * @param i 驱动器编号
 */
extern pFileEntry genRootFileEntry(int i);
/**
 * 将文件挂到其父目录的子文件列表中，用于file初始化。
 * 注意：只有初始化的时候，才需要提供pFileEntry结构和驱动器编号；初始化完成后，pFileEntry包含驱动器编号信息（方法是找到根目录）。
 * @param i 驱动器编号
 */
extern BOOL attachParent(pFileEntry file, int i);
/**
 * 建立父子目录间的双向指针连接
 */
extern void addChildren(pFileEntry parent, pFileEntry file, int i);

extern pFileEntry findFile(KEY frn,KEY pfrn,int i);

/**
 * 文件删除时，同时删除对应的内存结构
 */
extern void deleteFile(pFileEntry file);

/**
 * 启动文件系统变动监视线程。该线程只能被启动一次。
 * @param i 驱动器编号
 */
extern BOOL StartMonitorThread(int i);

/**
 * 停止文件系统变动监视线程。
 * @param i 驱动器编号
 */
extern BOOL StopMonitorThread(int i);

extern BOOL CloseVolumeHandle(int i);

/**
 * 根据文件的Key值在Map中找到该文件
 * @param frn 对于NTFS是文件的FileReferenceNumber,对于FAT是pFileEntry的内存地址值
 * @param i 驱动器编号
 */
extern pFileEntry findDir(KEY frn,int i);

/**
 * 初始化时将文件加入Map中。
 */
extern void add2Map(pFileEntry file,int i);
/**
 * 将Map内容清空，用于初始化时尝试读入了错误的数据库文件
 */
extern void resetMap(int i);

/**
 * 根据父目录KEY值建立目录树。
 * @param i 驱动器编号
 */
extern void build_dir(int i);

/**
 * 在迭代中访问一个文件
 * @param file 待访问的文件
 * @param data 环境数据
 */
typedef void (*pFileVisitor)(pFileEntry file, void *data);
typedef void (*pFileVisitor_p3)(pFileEntry file, WCHAR *full_name, int name_len, int i);
typedef BOOL (*pFileVisitorB)(pFileEntry file, void *data);

/**
 * 访问一个目录下的所有的文件，执行给定的访问函数
 * @param file 待遍历的目录
 * @param visitor 访问函数
 * @param data 环境数据
 */
extern void SubDirIterate(pFileEntry dir, pFileVisitor visitor, void *data);
extern void SubDirIterate_p3(pFileEntry dir, pFileVisitor_p3 visitor, WCHAR *full_name, int name_len, int i);
extern pFileEntry SubDirIterateB(pFileEntry dir, pFileVisitorB visitor, void *data);
/**
 * 递归遍历访问一个目录以及该目录下的所有的文件，执行给定的访问函数
 * @param file 待遍历的目录
 * @param visitor 访问函数
 * @param data 环境数据
 */
extern void FilesIterate(pFileEntry file,pFileVisitor visitor, void *data);
/**
 * 递归遍历访问当前文件系统的所有文件，执行给定的访问函数。
 * @param visitor 访问函数
 * @param data 环境数据
 */
extern void AllFilesIterate(pFileVisitor visitor, void *data);

extern void FileRemoveFilter(pFileEntry file, void *data);

extern BOOL check_file_entry(pFileEntry file, void *data);

#endif  // FILE_SEARCH_FS_COMMON_H_

#ifdef __cplusplus
}
#endif
