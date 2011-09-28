#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_DESKTOP_H_
#define FILE_SEARCH_DESKTOP_H_

#include "env.h"


/**
 * 根据Shell文件的DisplayName打开它
 * @param str 文件全路径名如“\\我的电脑\\控制面板\\键盘”
 */
extern BOOL exec_desktop(const wchar_t *str);


#endif  // FILE_SEARCH_DESKTOP_H_

#ifdef __cplusplus
}
#endif
