#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_ERROR_H_
#define FILE_SEARCH_ERROR_H_

#include "env.h"

extern void	 err_dump(const char *, ...);
extern void	 err_msg(const char *, ...);
extern void	 err_quit(const char *, ...);
extern void	 err_ret(const char *, ...);
extern void	 err_sys(const char *, ...);


#endif  // FILE_SEARCH_ERROR_H_

#ifdef __cplusplus
}
#endif
