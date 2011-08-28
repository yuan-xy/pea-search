#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_DESKTOP_H_
#define FILE_SEARCH_DESKTOP_H_

#include "env.h"
#include "fs_common.h"


/**
 * 扫描Shell命名空间，从根位置Desktop开始。
 */
extern void scan_desktop();

extern pFileEntry get_desktop();

#endif  // FILE_SEARCH_DESKTOP_H_

#ifdef __cplusplus
}
#endif
