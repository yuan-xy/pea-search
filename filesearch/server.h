#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_SERVER_H_
#define FILE_SEARCH_SERVER_H_

#include "env.h"
#include "sharelib.h"

#ifdef WIN32
	#define SockOut HANDLE
#else
	#define SockOut int	
#endif

extern void process(SearchRequest req, SockOut out);

extern BOOL start_named_pipe();

extern void wait_stop_named_pipe();

extern BOOL WINAPI shutdown_handle(DWORD CtrlEvent);

#endif  // FILE_SEARCH_SERVER_H_

#ifdef __cplusplus
}
#endif
