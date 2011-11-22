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
extern int scanUnix(pFileEntry root, int i);


#endif  // FILE_SEARCH_MAC_H_

#ifdef __cplusplus
}
#endif
