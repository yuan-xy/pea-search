#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_HISTORY_H_
#define FILE_SEARCH_HISTORY_H_

#include "env.h"

#define MAX_HISTORY 20

extern void history_add(const wchar_t *file);

extern BOOL history_save();

extern BOOL history_load();

typedef void (*pHistoryVisitor)(wchar_t *file, void *context);

extern void HistoryIterator(pHistoryVisitor, void *context);

extern int history_to_json(wchar_t *buffer);

#endif  // FILE_SEARCH_HISTORY_H_

#ifdef __cplusplus
}
#endif
