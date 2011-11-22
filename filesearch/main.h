#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_MAIN_H_
#define FILE_SEARCH_MAIN_H_

#include "env.h"

extern BOOL gigaso_init();
extern BOOL gigaso_destory();
    
#ifdef WIN32
extern void scan(int i);
extern void rescan(int i);
extern BOOL read_build_check(int i);
extern void after_build(int i);
#endif  // WIN32
    
    
#endif  // FILE_SEARCH_MAIN_H_

#ifdef __cplusplus
}
#endif