#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_SERVERNP_H_
#define FILE_SEARCH_SERVERNP_H_

extern BOOL start_named_pipe();
extern void wait_stop_named_pipe();
extern void shutdown_NP();
extern BOOL WINAPI shutdown_handle(DWORD CtrlEvent);

DWORD WINAPI  Hotkey(PVOID pParam);//系统热键监听线程，用于快速启动查询客户端

#endif  // FILE_SEARCH_SERVERNP_H_

#ifdef __cplusplus
}
#endif
