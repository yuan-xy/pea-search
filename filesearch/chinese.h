#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_CHINESE_H_
#define FILE_SEARCH_CHINESE_H_


#include "env.h"

#define PINYIN_CLASS_LEN 406

extern char pinyins[PINYIN_CLASS_LEN][6]; //所有的拼音
extern int py_lens[PINYIN_CLASS_LEN] ; //对应位置的拼音的字节长度
extern int hz_lens[PINYIN_CLASS_LEN] ; //对应位置拼音的同音汉字的个数
extern pUTF8 hzs[PINYIN_CLASS_LEN];    //汉字列表
extern int* hzs_pre_bndm[PINYIN_CLASS_LEN];    //同音汉字的bndm算法预处理结果数组的指针

extern char mohu_shenmu[6][2][3];
extern char mohu_yunmu[5][2][4];
extern char mohu_yunmu2[2][2][5];
#define DEFAULT_MOHU_SM_COUNT 4
#define DEFAULT_MOHU_YM_COUNT 3


extern BOOL init_chinese();

extern void print_hz_len(pUTF8 hz, int len);
extern void print_hz(pUTF8 hz);

extern void *parse_pinyin(char *str);
extern void *parse_pinyin1(char *str, int len);
extern void *parse_pinyin_and_pre_bndm(char *str, int len);
extern void free_parse_pinyin(void *p);

typedef void (*hz_visitor)(int names[], int name_len, pFileEntry file, BOOL *flag);

extern void hz_iterate(void *pinyin, hz_visitor visitor, pFileEntry file, BOOL *flag);

extern int hz_match_one(int index, pUTF8 str, int strlen);

#endif  // FILE_SEARCH_CHINESE_H_

#ifdef __cplusplus
}
#endif
