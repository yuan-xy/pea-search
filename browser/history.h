#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_HISTORY_H_
#define FILE_SEARCH_HISTORY_H_

#include "env.h"

#define VIEW_HISTORY 15  //首页显示的历史文件记录
#define MAX_HISTORY 30  //最大保存的历史文件记录

/**
* 将文件file添加到历史文件记录中。如果file在首页中已存在，则不重复添加。
*/
extern BOOL history_add(const wchar_t *file);

/**
* 删除给定index位置的历史文件记录, 将下一个位置的历史文件记录补充到被删除的位置
*/
extern void history_delete(int index);

/**
* 固定给定index位置的历史文件记录
*/
extern void history_pin(int index);
/**
* 取消固定给定index位置的历史文件记录
*/
extern void history_unpin(int index);

/**
* 获得给定index位置的历史文件记录
*/
extern wchar_t *history_get(int index);

/**
* 将历史文件记录保存到磁盘
*/
extern BOOL history_save();

/**
* 从磁盘文件中加载历史文件记录
*/
extern BOOL history_load();

extern BOOL history_remove();

typedef void (*pHistoryVisitor)(wchar_t *file, int pin, void *context);

/**
* 遍历首页的历史文件记录
*/
extern void HistoryIterator(pHistoryVisitor, void *context);

/**
* 遍历所有的历史文件记录
*/
extern void HistoryIteratorAll(pHistoryVisitor, void *context);

/**
* 将的历史文件记录信息导出为json格式
*/
extern int history_to_json(wchar_t *buffer);

#endif  // FILE_SEARCH_HISTORY_H_

#ifdef __cplusplus
}
#endif
