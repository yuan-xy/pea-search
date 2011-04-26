#include "env.h"
#include <stdio.h>
#include <string.h>
#include "chinese.h"
#include "util.h"
#include <vector>

typedef std::vector<int> IntList, *pIntList;

typedef std::vector<pIntList> WordList, *pWordList;


extern "C" {

void print_pinyin_parse(pWordList words){
	if(words == NULL) return;
	for(WordList::const_iterator it = words->begin(); it!= words->end(); ++it) {
		pIntList list = pIntList(*it);
		for(IntList::const_iterator pinyin = list->begin(); pinyin!= list->end(); ++pinyin) {
			int index = (int)(*pinyin);
			printf("%s-",pinyins[index]);
		}
		printf("\n");
	}
}

static int match_pinyin(char *str, int len,int start_index){
	int i;
	for(i=start_index;i<PINYIN_CLASS_LEN;i++){
		if(py_lens[i]<=len && strncmp(str,pinyins[i],py_lens[i])==0) return i;
	}
	return -1;
}

static int exact_match_pinyin(char *str, int len){
	int i;
	for(i=0;i<PINYIN_CLASS_LEN;i++){
		if(py_lens[i]==len && strncmp(str,pinyins[i],py_lens[i])==0) return i;
	}
	return -1;
}

static void parse_pinyin_1(char *str, int len, pWordList words, pIntList intlist){
	int i=0,j=0;
	int k = (int)strcspn(str,"\'");
	if(k>0){
		int t1=exact_match_pinyin(str,k);
		int t2=exact_match_pinyin(str+k+1,len-k-1);
		if(t1 !=-1 && t2 !=-1){
			pIntList newlist = new IntList;
			newlist->push_back(t1);
			newlist->push_back(t2);
			words->push_back(newlist);
			return;
		}
	}
	while((i=match_pinyin(str,len,j))!=-1){
		j=i+1;
		pIntList newlist = new IntList;
		int count=0;
		for(;count< (int)intlist->size();count++){
			newlist->push_back(intlist->at(count));
		}
		newlist->push_back(i);
		if(py_lens[i]==len){
			words->push_back(newlist);
			continue;
		}else{
			parse_pinyin_1(str+py_lens[i], len-py_lens[i], words,newlist);
			continue;
		}
	}
}

static void add_mohu(pWordList words, pIntList list, int index, int pinyin){
	pIntList newlist = new IntList(*list);
	(*newlist)[index] = pinyin;
	words->push_back(newlist);
	//printf("mohu: %s\n",pinyins[pinyin]);
}

static void mohu_01(pWordList words, pIntList list, int ii, BOOL shortpy){
	int index = list->at(ii);
	char *pinyin = pinyins[index];
	int pinyin_len = (int)strlen(pinyin);
	int i, s_index, l_index;
	if(shortpy){
		s_index = 0;
		l_index = 1;
	}else{
		s_index = 1;
		l_index = 0;
	}
	for(i=0;i<DEFAULT_MOHU_SM_COUNT;i++){
		char *p0 = mohu_shenmu[i][s_index];
		int len0 = (int)strlen(p0);
		if(len0<pinyin_len && strncmp(p0,pinyin,len0)==0){
			char mh[8];
			char *p1 = mohu_shenmu[i][l_index];
			int len1 = (int)strlen(p1);
			int k;
			strncpy(mh,p1,len1);
			strncpy(mh+len1,pinyin+len0,pinyin_len-len0);
			mh[7]='\0';
			if((k=exact_match_pinyin(mh,pinyin_len-len0+len1))!=-1){
				add_mohu(words,list,ii,k);
			}
		}
	}
	for(i=0;i<DEFAULT_MOHU_YM_COUNT;i++){
		char *p0 = mohu_yunmu[i][s_index];
		int len0 = (int)strlen(p0);
		if(len0<pinyin_len && strncmp(p0,pinyin+pinyin_len-len0,len0)==0){
			char mh[8];
			char *p1 = mohu_yunmu[i][l_index];
			int len1 = (int)strlen(p1);
			int k;
			strncpy(mh,pinyin,pinyin_len-len0);
			strncpy(mh+pinyin_len-len0,p1,len1);
			mh[7]='\0';
			if((k=exact_match_pinyin(mh,pinyin_len-len0+len1))!=-1){
				add_mohu(words,list,ii,k);
			}
		}
	}
}

static void mohu(char *str, int len,pWordList words){
	int i,col;
	for(col=0;col<len;col++){
		int size = (int)words->size();
		for(i=0;i<size;i++){
			pIntList list = words->at(i);
			if(col < (int)list->size()){
				mohu_01(words,list,col, 1);
				mohu_01(words,list,col, 0);
			}
		}
	}
}

pWordList parse_pinyin0(char *str, int len){
	if(len==0) return NULL;
	pWordList words = new WordList;
	pIntList intlist = new IntList;
	parse_pinyin_1(str,len,words,intlist);
	if(words->empty()) return NULL;
	mohu(str,len,words);
	return words;
}

void free_parse_pinyin(void *p){
	if(p==NULL) return;
	pWordList ret = (pWordList)p;
	ret->clear();
}




void *parse_pinyin1(char *str, int len){
	pWordList words = parse_pinyin0(str,len);
	return (void *)words;
}

void *parse_pinyin(char *str){
	pWordList ret = parse_pinyin0(str,(int)strlen(str));
	print_pinyin_parse(ret);
	return (void *)ret;
}

extern void pre_bndm_cn(int index);

void *parse_pinyin_and_pre_bndm(char *str, int len){
	pWordList words = parse_pinyin0(str,len);
	if(words==NULL) return NULL;
	for(WordList::const_iterator it = words->begin(); it!= words->end(); ++it) {
		pIntList list = pIntList(*it);
		for(unsigned int i=0;i<list->size();i+=2){//汉字按双字匹配，第一个字用bndm算法匹配，找到以后下一个字用暴力搜索。所以每两个字只需要预处理第一个字
			int index = (*list)[i];
			if(hzs_pre_bndm[index]==NULL) pre_bndm_cn(index);
		}
	}
	return (void *)words;
}

void hz_iterate(void *pinyin, hz_visitor visitor, pFileEntry file, BOOL *flag){
	pWordList words = (pWordList)pinyin;
	if(words == NULL) return;
	for(WordList::const_iterator it = words->begin(); it!= words->end(); ++it) {
		pIntList list = pIntList(*it);
		int names[16];
		int i=0,j=0;
		for(IntList::const_iterator pinyin = list->begin(); pinyin!= list->end(); ++pinyin) {
			int index = (int)(*pinyin);
			names[i++]=index;
		}
		visitor(names,i,file, flag);
		if((*flag)) return;
	}
}
	
}// extern "C"
