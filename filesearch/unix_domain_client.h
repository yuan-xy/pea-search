#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_UD_CLIENT_H_
#define FILE_SEARCH_UD_CLIENT_H_

#include "env.h"
#include "sharelib.h"

extern	BOOL connect_unix_socket(int *psock);

extern	BOOL query(int sockfd, SearchRequest *req, char *buffer);

#endif  // FILE_SEARCH_UD_CLIENT_H_

#ifdef __cplusplus
}
#endif

