#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_COMMON_H_
#define FILE_SEARCH_COMMON_H_

#include "env.h"

/**
 * 设置应用程序当前目录
 * @param lpModuleName 如果lpModuleName为空， 则设置应用程序路径为其exe文件所在目录。如果lpModuleName不为空，一般是浏览器中加载DLL，设置为DLL所在路径
 */
extern BOOL setPWD(char *lpModuleName);

/**
 * 设置应用程序当前目录为用户主目录
 */
extern BOOL setUserPWD();

/**
 * 以进程当前工作目录为参照，根据相对路径文件名得到绝对路径名
 * @param name 相对于本模块的文件名
 * @param full_path 绝对路径名
 */
extern BOOL get_abs_path(const WCHAR *name, WCHAR full_path[]);

/**
 * 以当前可执行文件所在目录为参照，根据相对路径文件名得到绝对路径名
 * @param name 相对于本模块的文件名
 * @param full_path 绝对路径名
 */
extern BOOL get_abs_path_exe(const WCHAR *name, WCHAR full_path[]);

extern void print_debug(WCHAR *fmt, ...);

extern BOOL WindowsVersionOK();


extern BOOL get_os(WCHAR *osbuf);

extern BOOL get_cpu(WCHAR *cpubuf);

extern BOOL get_disk(WCHAR *diskbuf);

extern BOOL get_ver(WCHAR *verbuf);

extern BOOL get_user(WCHAR *userbuf);

extern BOOL is_admin(void);

extern BOOL passed_one_day(time_t last);

extern BOOL file_passed_one_day(char *filename);

extern int wchar_to_utf8_len(const WCHAR *in, int insize_c);
extern void wchar_to_utf8_nocheck(const WCHAR *in, int insize_c, pUTF8 out, int out_size);

extern int utf8_to_wchar_len(const pUTF8 in, int insize_b);
extern int utf8_to_wchar_nocheck(const pUTF8 in, int insize_b, WCHAR *out, int out_buffer_size);


#endif  // FILE_SEARCH_COMMON_H_

#ifdef __cplusplus
}
#endif
