#include "env.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h> 
#include "util.h"
#include "global.h"
#include "search.h"
#include "drive.h"
#include "suffix.h"
#include "str_match.h"
#include "fs_common.h"
#include "chinese.h"

#define AND_LOGIC 1 //空格分隔的搜索项
#define OR_LOGIC 2  // | 搜索项
#define NOT_LOGIC 3  // -搜索项

#define NORMAL_MATCH 4 //普通匹配
#define WHOLE_MATCH 5  // "ad sf", 以引号包括起来的字符串，其字符串包含特殊字符，完全匹配单词，不转义

#define WORD_MATCH 6  // "adsf", 以引号包括起来的字符串，其字符串不包含特殊字符，匹配单词（单词前后有分隔符）
#define BEGIN_MATCH 7 // "abc*",以abc开头
#define END_MATCH 8   // "*abc",以abc结尾
#define ALL_MATCH 9   // 特殊输入"*"，匹配所有文件
#define NO_SUFFIX_MATCH 10  //特殊输入"*."，匹配没有后缀名的文件，不包括文件夹

struct searchOpt{ //查询条件
	WCHAR *wname; //查询字符串
	FILE_NAME_LEN wlen; //查询字符串长度
	pUTF8 name; //查询字符串,utf8编码
	FILE_NAME_LEN len; //查询字符串长度
	unsigned char logic;  //查询结果组合逻辑
	unsigned char match_t; //查询字符串匹配方式
	struct searchOpt *next; //下一个查询条件
	struct searchOpt *subdir; //如果查询字符串包含路径，那么这里保存第一个路径分隔符号后面的查询条件
	void *pinyin; //拼音分解结果, pWordList指针
	int preStr[ASIZE+XSIZE]; //KMP、QS等算法的预处理字符串偏移
};
typedef struct searchOpt SearchOpt, *pSearchOpt;

static int count=0,matched=0; //所有查询的文件总数、最终符合条件的总数
static pFileEntry *list; //保存搜索结果列表
static SearchEnv defaultEnv={0}; //缺省查询环境
static pSearchEnv sEnv; //当前查询环境

static BOOL allowPinyin(pSearchOpt opt){ //大小写不敏感、一般匹配模式，模式串不包含中文
	return !sEnv->case_sensitive && opt->match_t==NORMAL_MATCH && opt->len==opt->wlen;
}

typedef struct{
	pUTF8 name;
	int cur_len;
} Strlen, *pStrlen;

INLINE static BOOL match(pFileEntry file, void *data){
	pStrlen strlen = (pStrlen)data;
	return (strnicmp(file->FileName,strlen->name,strlen->cur_len)==0);
}

static pFileEntry find_file_in(pFileEntry parent, pUTF8 name, int whole_len){
	pFileEntry tmp;
	Strlen strlen;
	int i, cur_len=whole_len;
	for(i=1;i<whole_len;i++){
		if(*(name+i)=='\\' || *(name+i)=='/'){
			cur_len=i;
			break;
		}
	}
	strlen.name = name;
	strlen.cur_len = cur_len;
	tmp = SubDirIterateB(parent,match,&strlen);
	if(tmp == NULL) return NULL;
	if(cur_len == whole_len || (cur_len == whole_len-1  && *(name+cur_len)=='\\') ) return tmp;
	return find_file_in(tmp, name+cur_len+1, whole_len-cur_len-1);
}

pFileEntry find_file(WCHAR *name, int len){
	int d;
	if(len==0) return NULL;
	d = toupper((char)*name)-'A';
	if(d<0 || d>25) return NULL;
	if(len<=3) return g_rootVols[d];
	if(*(name+1)==L':' && ( *(name+2)==L'\\' || *(name+2)==L'/' ) ){
		int ustrlen;
		pUTF8 ustr = wchar_to_utf8(name+3,len-3,&ustrlen);
		return find_file_in(g_rootVols[d],ustr,ustrlen);
	}
	return NULL;
}

BOOL hz_match_two(int index1,int index2,pFileEntry file){
	int offset=0;
	do{
		int len = hz_match_one(index1,file->FileName+offset,file->us.v.FileNameLength-offset);
		if(len){
			pUTF8 name2 = hzs[index2];
			int len2=0;
			offset +=len;
			for(len2=0;len2 < hz_lens[index2]*3;len2+=3){
				if(memcmp(name2+len2,file->FileName+offset,3)==0){
/*					print_hz_len(file->FileName+offset-3,3);
					print_hz_len(name2+len2,3);
					printf("\n");
					if(offset!=len){
						printf("first try not match and second try match.\n");
					}*/
					return 1;
				}
			}
		}else{
			return 0;
		}
	}while(offset < file->us.v.FileNameLength);
	return 0;
}

void hz_match(int names[], int name_len, pFileEntry file, BOOL *flag){		
	int couple = name_len/2, remainder = name_len%2, i;
	for(i=0;i<couple;i++){
		if(!hz_match_two(names[2*i],names[2*i+1],file)) return;
	}
	if(remainder==1){
		if(hz_match_one(names[name_len-1],file->FileName,file->us.v.FileNameLength)) (*flag) = 1;
	}else{
		(*flag) = 1;
	}
}

BOOL match_opt(pFileEntry file, pSearchOpt opt){
	BOOL match;
	if(opt->len > file->us.v.FileNameLength){
		match=0;
	}else{
		switch(opt->match_t){
			case ALL_MATCH: match = 1;break;
			case NO_SUFFIX_MATCH: match = (file->ut.v.suffixType==SF_NONE);break;
			case BEGIN_MATCH: match = begin_match(opt->name,opt->len,file->FileName,file->us.v.FileNameLength,sEnv->case_sensitive);break;
			case END_MATCH: match = end_match(opt->name,opt->len,file->FileName,file->us.v.FileNameLength,sEnv->case_sensitive);break;
			case WORD_MATCH:
					match = word_match(opt->name,opt->len,file->FileName,file->us.v.FileNameLength,sEnv->case_sensitive);
					break;
			case WHOLE_MATCH:
			default:
				match = SUBSTR(opt->name,opt->len,file->FileName,file->us.v.FileNameLength,opt->preStr,sEnv->case_sensitive);
				break;
		}
	}
	if(match) return match;
	if(opt->pinyin!=NULL){
		if(file->us.v.FileNameLength > file->us.v.StrLen){  //文件名包含中文
			hz_iterate(opt->pinyin, hz_match, file, &match); //中文拼音查询
		}
	}
	return match;
}

void FileSearchVisitor(pFileEntry file, void *data){
	pSearchOpt opt = (pSearchOpt)data;
	BOOL flag=1;
	if(sEnv->file_type!=0 && !include_type(sEnv->file_type,file->ut.v.suffixType)) goto no_match;
	do{
		BOOL match = match_opt(file, opt);
		if(match && opt->subdir!=NULL){
			if(IsDir(file)){
				pFileEntry tmp = find_file_in(file,opt->subdir->name, opt->subdir->len);
				if(tmp!=NULL){
					*(list++) = tmp;
					matched++;
				}
				goto no_match;
			}else{
				match = 0;
			}
		}
		switch(opt->logic){
			case AND_LOGIC: flag = flag && match;break;
			case OR_LOGIC: flag = flag || match;break;
			case NOT_LOGIC: flag = flag && (!match);break;
			default:		flag &=match;break;
		}
	}while((opt = opt->next)!=NULL);
	if(flag){
		*(list++) = file;
		matched++;
	}
no_match:
	count++;
}

INLINE int file_len_cmp(pFileEntry a, pFileEntry b){
	if(a==b) return 0;
	if(a==NULL) return -1;
	if(b==NULL) return 1;
	return a->us.v.FileNameLength - b->us.v.FileNameLength;
}

int file_name_cmpUTF8(pFileEntry a, pFileEntry b){
	int ret;
	if(a==b) return 0;
	if(a==NULL) return -1;
	if(b==NULL) return 1;
	{
		WCHAR aw[256],bw[256];
		int ai,bi;
		ai = MultiByteToWideChar(CP_UTF8, 0,a->FileName,a->us.v.FileNameLength, aw, 256);
		bi = MultiByteToWideChar(CP_UTF8, 0,b->FileName,b->us.v.FileNameLength, bw, 256);
		if(sEnv->case_sensitive){
			ret = _wcsncoll(aw,bw,min(ai,bi));
		}else{
			ret = _wcsnicoll(aw,bw,min(ai,bi));
		}
	}
	if(ret!=0)  return ret;
	return a->us.v.FileNameLength - b->us.v.FileNameLength;
}

INLINE int file_name_cmp(pFileEntry a, pFileEntry b){
	int ret;
	if(a==b) return 0;
	if(a==NULL) return -1;
	if(b==NULL) return 1;
	ret = strncmp(a->FileName,b->FileName,min(a->us.v.FileNameLength,b->us.v.FileNameLength));
	if(ret!=0)  return ret;
	return a->us.v.FileNameLength - b->us.v.FileNameLength;
}

int path_cmp(pFileEntry a, pFileEntry b){
	pFileEntry as[8];
	pFileEntry bs[8];
	int i=0,j=0;
	do{
		if(i<8){
			as[i++]=a;
		}else{
			as[7]=a;
		}
		a = a->up.parent;
	}while(a!=NULL);
	do{
		if(j<8){
			bs[j++]=b;
		}else{
			bs[7]=b;
		}
		b = b->up.parent;
	}while(b!=NULL);
	for(i-=1,j-=1;;i--,j--){
		int ret;
		if(i<0 || j<0) break;
		ret = file_name_cmp(as[i],bs[j]);
		if(ret!=0)  return ret;
	}
	return i-j;
}

int path_cmp2(pFileEntry pa, pFileEntry pb){
	pFileEntry as[32];
	pFileEntry bs[32];
	int i=0,j=0;
	pFileEntry a = pa;
	pFileEntry b = pb;
	do{
		as[i++]=a;
		a = a->up.parent;
	}while(a!=NULL);
	do{
		bs[j++]=b;
		b = b->up.parent;
	}while(b!=NULL);
	for(i-=1,j-=1;;i--,j--){
		int ret;
		if(i<0 || j<0) break;
		ret = file_name_cmp(as[i],bs[j]);
		if(ret!=0)  return ret;
	}
	return i-j;
}

int order_compare(const void *pa, const void *pb){
	pFileEntry a,b;
	a = *(pFileEntry *)pa;
	b = *(pFileEntry *)pb;
	switch(sEnv->order){
		case DATE_ORDER_ASC:
			return GET_TIME(a) - GET_TIME(b);
		case DATE_ORDER_DESC:
			return GET_TIME(b) - GET_TIME(a);
		case SIZE_ORDER_ASC:
			return GET_SIZE(a) - GET_SIZE(b);
		case SIZE_ORDER_DESC:
			return GET_SIZE(b) - GET_SIZE(a);
		case PATH_ORDER_ASC:
			return path_cmp2(a,b);
		case PATH_ORDER_DESC:
			return path_cmp2(b,a);
		case NAME_ORDER_DESC:
			return file_name_cmpUTF8(b,a);
		case NAME_ORDER_ASC:
		default:
			return file_name_cmp(a,b);
	}
}



static void trim(SearchOpt *opt){
	WCHAR *p1=opt->wname, *p2=opt->wname+opt->wlen-1;
	if(opt->match_t == WHOLE_MATCH) return;
	while( iswspace(*p1) ){
		if( (p1-opt->wname)>=opt->wlen) break;
		p1++;
	}
	while( iswspace(*p2) ){
		if( (p2-opt->wname)<=0) break;
		p2--;
	}
	opt->wname=p1;
	opt->wlen=(FILE_NAME_LEN)(p2-p1+1);
}

static void assign(SearchOpt *sOpt, WCHAR *str){
	sOpt->wlen = (FILE_NAME_LEN) wcslen(str);
	sOpt->wname = str;
	sOpt->logic = AND_LOGIC;
	sOpt->next = NULL;
}

static void processOr(SearchOpt *sOpt){
	SearchOpt *s0 = sOpt;
	WCHAR *p1=s0->wname;
	if(sOpt->match_t != WHOLE_MATCH){
		while( (p1-s0->wname) < s0->wlen ){
			if( (*p1 == L'|') || (*p1 == L'｜') ){
				NEW0(SearchOpt,s1);
				s1->wlen = (FILE_NAME_LEN)(s0->wlen-(p1-s0->wname)-1);
				s1->wname = p1+1;
				s1->logic = OR_LOGIC;
				s1->next = s0->next;
				s0->wlen = (FILE_NAME_LEN)(p1-s0->wname);
				s0->next = s1;
				trim(s0);
				trim(s1);
				if(s1->wlen==0 && s1->next!=NULL){
					s1 = s1->next;
					s1->logic = OR_LOGIC;
				}
				s0 = s1;
				p1 = s0->wname;
			}else{
				p1++;
			}
		}
	}
	if(s0->next!=NULL) processOr(s0->next);
}

static void processAnd(SearchOpt *sOpt){
	SearchOpt *s0 = sOpt;
	WCHAR *p1=s0->wname;
	if(sOpt->match_t != WHOLE_MATCH){
		while( (p1-s0->wname) < s0->wlen ){
			if(iswspace(*p1)){
				NEW0(SearchOpt,s1);
				s1->wlen = (FILE_NAME_LEN)(s0->wlen-(p1-s0->wname)-1);
				s1->wname = p1+1;
				s1->logic = AND_LOGIC;
				s1->next = s0->next;
				s0->wlen = (FILE_NAME_LEN)(p1-s0->wname);
				s0->next = s1;
				trim(s0);
				trim(s1);
				s0 = s1;
				p1 = s0->wname;
			}else{
				p1++;
			}
		}
	}
	if(s0->next!=NULL) processAnd(s0->next);
}

static void processNot(SearchOpt *s0){
	while(s0!=NULL){
		if( (*s0->wname == L'-' || *s0->wname == L'－') && s0->match_t != WHOLE_MATCH){
			s0->wname +=1;
			s0->wlen -=1;
			s0->logic = NOT_LOGIC;
		}
		s0 = s0->next;
	}
}

static void splitAterisk(SearchOpt *s0,int i){	// abc*def
							NEW0(SearchOpt,s1);
							s1->wlen = s0->wlen-i-1;
							s1->wname = s0->wname+i+1;
							s1->logic = AND_LOGIC;
							s1->match_t = END_MATCH;
							s1->next = s0->next;
							s0->wname +=1;
							s0->wlen =i-1;
							s0->match_t = NORMAL_MATCH;
							s0->next = s1;
}

static void processAterisk(SearchOpt *s0){
	while(s0!=NULL){
		if(*s0->wname == L'*'){
			if(s0->wlen==1){					   // *
				s0->match_t = ALL_MATCH;
			}else if(*(s0->wname+1) == L'.' && s0->wlen==2){	   // *.
				s0->match_t = NO_SUFFIX_MATCH;
			}else if(*(s0->wname+1) == L'*' && s0->wlen==2){		   // **
				s0->match_t = ALL_MATCH;
			}else if(s0->match_t != WHOLE_MATCH){
				int i=2;
				for(;i<s0->wlen;i++){
					if(*(s0->wname+i) == L'*'){
						if(i+1==s0->wlen){					   // *abc*
							s0->wname +=1;
							s0->wlen =i-1;
							s0->match_t = NORMAL_MATCH;
						}else{								// *abc*efg
							splitAterisk(s0,i);
						}
						goto next;
					}
				}									// *abcefg
				s0->wname +=1;
				s0->wlen -=1;
				s0->match_t = END_MATCH;
			}
		}else if(*(s0->wname+s0->wlen-1) == L'*' && s0->match_t != WHOLE_MATCH){ // abc*
			s0->wlen -=1;
			s0->match_t = BEGIN_MATCH;
		}else if(s0->match_t != WHOLE_MATCH){
			int i=1;
			for(;i<s0->wlen-1;i++){
				if(*(s0->wname+i)==L'*'){							// abc*def
					splitAterisk(s0,i);
					goto next;
				}
			}
		}
next:
		s0 = s0->next;
	}
}

static void checkLen(SearchOpt *s0){
	while(s0!=NULL && s0->next!=NULL){
		if(s0->next->wlen==0){
			s0->next = s0->next->next;
		}
		s0 = s0->next;
	}
}

static void checkMatch(SearchOpt *s0){
	while(s0!=NULL){
		if(s0->match_t<NORMAL_MATCH || s0->match_t>NO_SUFFIX_MATCH){
			s0->match_t = NORMAL_MATCH;
		}
		s0 = s0->next;
	}
}

static void wordMatch(SearchOpt *s0){
	while(s0!=NULL){
		if(s0->match_t == WHOLE_MATCH){
			int i=0;
			for(;i<s0->wlen;i++){
				if(!iswalpha(s0->wname[i])) goto here;
			}
			s0->match_t = WORD_MATCH;
		}
here:
		s0 = s0->next;
	}
}

static void findSlash(SearchOpt *s0){
	while(s0!=NULL){
		if(s0->match_t == NORMAL_MATCH || s0->match_t == BEGIN_MATCH){
			int i=0,len=s0->wlen;
			for(;i<len;i++){
				if(*(s0->wname+i)==L'/' || *(s0->wname+i)==L'\\'){
					NEW0(SearchOpt,s1);
					s1->wlen = s0->wlen-i-1;
					s1->wname = s0->wname+i+1;
					s1->next = NULL;
					s0->wlen =i-1;
					s0->subdir = s1;
					return;
				}
			}
			return;
		}
		s0 = s0->next;
	}
}

static void wholeMatch(SearchOpt *s0){
	WCHAR *p1,*p2;
	if((p1=wcschr(s0->wname,L'"'))!=NULL){
		p2=wcschr(p1+1,L'"');
		if(p1!=s0->wname && p2==NULL){
			NEW0(SearchOpt,s1);
			s1->wlen = (FILE_NAME_LEN)(s0->wlen-(p1-s0->wname)-1);
			s1->wname = p1+1;
			s1->logic = AND_LOGIC;
			s1->next = NULL;
			s0->wlen = (FILE_NAME_LEN)(p1-s0->wname);
			s0->next = s1;
			trim(s0);
			trim(s1);
		}else if(p1!=s0->wname && p2!=NULL){
			NEW(SearchOpt,s1);
			NEW0(SearchOpt,s2);
			memset(s1,0,sizeof(SearchOpt));
			s1->wlen = (FILE_NAME_LEN)(p2-p1-1);
			s1->wname = p1+1;
			s1->logic = AND_LOGIC;
			s1->next = s2;
			s2->wlen = (FILE_NAME_LEN)(s0->wlen-(p2-s0->wname)-1);
			s2->wname = p2+1;
			s2->logic = AND_LOGIC;
			s2->next = NULL;
			s0->wlen = (FILE_NAME_LEN)(p1-s0->wname);
			s0->next = s1;
			trim(s0);
			s1->match_t = WHOLE_MATCH;
			trim(s2);
		}else if(p1==s0->wname && p2!=NULL){
			NEW0(SearchOpt,s1);
			s1->wlen = (FILE_NAME_LEN)(s0->wlen-(p2-s0->wname)-1);
			s1->wname = p2+1;
			s1->logic = AND_LOGIC;
			s1->next = NULL;
			s0->wlen = (FILE_NAME_LEN)(p2-p1-1);
			s0->wname += 1;
			s0->next = s1;
			s0->match_t = WHOLE_MATCH;
			trim(s1);
		}else if(p1==s0->wname && p2==NULL){
			s0->wlen -= 1;
			s0->wname += 1;
		}else{
			my_assert(0, );
		}
	}
}

static void toUTF8(SearchOpt *s0){
	while(s0!=NULL){
		int len;
		s0->name = wchar_to_utf8(s0->wname,s0->wlen,&len);
		s0->len = len;
		s0 = s0->next;
	}
}

static void preProcessSearchOpt(SearchOpt *s0){
	while(s0!=NULL){
		if(s0->match_t==NORMAL_MATCH || s0->match_t==WHOLE_MATCH ){
			preProcessPattern(s0->name,s0->len,s0->preStr,sEnv->case_sensitive);
		}
		s0 = s0->next;
	}
}

static void preProcessPinyin(SearchOpt *s0){
	while(s0!=NULL){
		if( allowPinyin(s0)){
			s0->pinyin = parse_pinyin_and_pre_bndm(s0->name,s0->len);
		}
		s0 = s0->next;
	}
}

static void genSearchOpt(SearchOpt *s0,WCHAR *str2){
	assign(s0, str2);
	trim(s0);
	wholeMatch(s0);
	processOr(s0);
	processAnd(s0);
	processNot(s0);
	processAterisk(s0);
	wordMatch(s0);
	checkLen(s0);
	checkMatch(s0);
	findSlash(s0);
	toUTF8(s0);
}

static void freeSearchOpt(SearchOpt *s0){
	while(s0!=NULL){
		SearchOpt *next = s0->next;
		free_safe(s0->name);
		free_parse_pinyin(s0->pinyin);
		free_safe(s0);
		s0 = next;
	}
}

BOOL nullString(WCHAR *str){
	int i;
	for(i=0;i<(int)wcslen(str);i++){
		WCHAR c = *(str+i);
		if(!iswspace(c)) return 0;
	}
	return 1;
}

DWORD search(WCHAR *str, pSearchEnv env, pFileEntry **result){
	if(nullString(str)){
		return 0;
	}else{
		pFileEntry dir=NULL;
		NEW0(SearchOpt,sOpt);
		genSearchOpt(sOpt,str);
		count=0,matched=0;
		if(env==NULL){
			sEnv = &defaultEnv;
		}else{
			sEnv = env;
			dir = find_file(env->path_name,env->path_len);
		}
		preProcessSearchOpt(sOpt);
		preProcessPinyin(sOpt);
		list = (pFileEntry *)malloc_safe(sizeof(pFileEntry)*ALL_FILE_COUNT);
		//TODO: 何时释放内存
		*result = list;
		if(dir != NULL){
			FilesIterate(dir,FileSearchVisitor,sOpt);
		}else{
			AllFilesIterate(FileSearchVisitor,sOpt);
		}
		freeSearchOpt(sOpt);
		if(sEnv->order>0 || matched<1000){
			if(sEnv->order==0 ) sEnv->order=NAME_ORDER_ASC;
			qsort(*result,matched,sizeof(pFileEntry),order_compare);
		}
		printf("all: %d, matched:%d\n",count,matched);
		return matched;
	}
}

void free_search(){
	free_safe(list);
}
