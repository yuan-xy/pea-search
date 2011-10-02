#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_EXCEPTION_DUMP_H_
#define FILE_SEARCH_EXCEPTION_DUMP_H_

/**
* 初始化breakpad
*/
extern void breakpad_init();

/**
* 无需异常，生成一个dump
*/
extern BOOL request_dump();


#endif  // FILE_SEARCH_EXCEPTION_DUMP_H_

#ifdef __cplusplus
}
#endif