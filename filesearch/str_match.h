#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_STR_MATCH_H_
#define FILE_SEARCH_STR_MATCH_H_

#include "env.h"

#define ASIZE 256  //文件名字符集大小
#define XSIZE 256  //查询串字符集大小

/**
 * 使用quick-search or KMP算法判断x是否是y的子串
 * @param x 模式串
 * @param m x的长度
 * @param y 被匹配串
 * @param n y的长度
 * @param case_sensitive 是否大小写敏感
 */
extern  BOOL SUBSTR(pUTF8 x, int m, pUTF8 y, int n, int preNext[], BOOL case_sensitive);
extern  void preProcessPattern(pUTF8 x, int m, int preNext[], BOOL case_sensitive);

extern void pre_bndm_hz_multi(pUTF8 x, int m, int *prestr);
extern int bndm_hz_multi(pUTF8 x, int m, pUTF8 y, int n, int *preNext);

/**
 * 使用暴力算法判断x是否是y的子串
 */
extern brute_force(pUTF8 x, int m, pUTF8 y, int n, BOOL case_sensitive);

/**
 * 判断y串中是否包含独立的单词x
 */
extern  BOOL word_match(pUTF8 x, int m, pUTF8 y, int n, BOOL case_sensitive);

/**
 * 判断y串中是否以x开头
 */
extern BOOL begin_match(pUTF8 x, int m, pUTF8 y, int n, BOOL case_sensitive);
/**
 * 判断y串中是否以x结尾
 */
extern BOOL end_match(pUTF8 x, int m, pUTF8 y, int n, BOOL case_sensitive);

#endif  // FILE_SEARCH_STR_MATCH_H_

#ifdef __cplusplus
}
#endif
