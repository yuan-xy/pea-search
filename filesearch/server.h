#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_SERVER_H_
#define FILE_SEARCH_SERVER_H_

#include "env.h"
#include "sharelib.h"

extern void process(SearchRequest req, void *out);

#endif  // FILE_SEARCH_SERVER_H_

#ifdef __cplusplus
}
#endif
