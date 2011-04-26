#include "env.h"
#include "util.h"
#include <stdio.h>
#include "chinese.h"
#include "str_match.h"

/* 汉字字库参考freepy，取消下列汉字
den     扥扽
eng	鞥
m	呒呣嘸
n	嗯咹哏
ng	嗯
o	哦喔噢筽
rua	挼
 */

char pinyins[PINYIN_CLASS_LEN][6] = {"a","ai","an","ang","ao","ba","bai","ban","bang","bao","bei","ben","beng","bi","bian","biao","bie","bin","bing","bo",
		"bu","ca","cai","can","cang","cao","ce","cen","ceng","cha","chai","chan","chang","chao","che","chen","cheng","chi","chong",
		"chou","chu","chuai","chuan","chuang","chui","chun","chuo","ci","cong","cou","cu","cuan","cui","cun","cuo","da","dai",
		"dan","dang","dao","de","dei","deng","di","dia","dian","diao","die","ding","diu","dong","dou","du","duan","dui",
		"dun","duo","e","ei","en","er","fa","fan","fang","fei","fen","feng","fo","fou","fu","ga","gai","gan","gang","gao",
		"ge","gei","gen","geng","gong","gou","gu","gua","guai","guan","guang","gui","gun","guo","ha","hai","han","hang","hao",
		"he","hei","hen","heng","hong","hou","hu","hua","huai","huan","huang","hui","hun","huo","ji","jia","jian","jiang","jiao",
		"jie","jin","jing","jiong","jiu","ju","juan","jue","jun","ka","kai","kan","kang","kao","ke","kei","ken","keng","kong",
		"kou","ku","kua","kuai","kuan","kuang","kui","kun","kuo","la","lai","lan","lang","lao","le","lei","leng","li","lia",
		"lian","liang","liao","lie","lin","ling","liu","lo","long","lou","lu","luan","lue","lun","luo","lv","ma","mai",
		"man","mang","mao","me","mei","men","meng","mi","mian","miao","mie","min","ming","miu","mo","mou","mu","na","nai",
		"nan","nang","nao","ne","nei","nen","neng","ni","nian","niang","niao","nie","nin","ning","niu","nong","nou","nu",
		"nuan","nue","nuo","nv","ou","pa","pai","pan","pang","pao","pei","pen","peng","pi","pian","piao","pie","pin","ping",
		"po","pou","pu","qi","qia","qian","qiang","qiao","qie","qin","qing","qiong","qiu","qu","quan","que","qun","ran","rang",
		"rao","re","ren","reng","ri","rong","rou","ru","ruan","rui","run","ruo","sa","sai","san","sang","sao","se","sen",
		"seng","sha","shai","shan","shang","shao","she","shei","shen","sheng","shi","shou","shu","shua","shuai","shuan","shuang",
		"shui","shun","shuo","si","song","sou","su","suan","sui","sun","suo","ta","tai","tan","tang","tao","te","teng","ti",
		"tian","tiao","tie","ting","tong","tou","tu","tuan","tui","tun","tuo","tei","wa","wai","wan","wang","wei","wen","weng",
		"wo","wu","xi","xia","xian","xiang","xiao","xie","xin","xing","xiong","xiu","xu","xuan","xue","xun","ya","yan","yang",
		"yao","ye","yi","yin","ying","yo","yong","you","yu","yuan","yue","yun","za","zai","zan","zang","zao","ze","zei","zen",
		"zeng","zha","zhai","zhan","zhang","zhao","zhe","zhei","zhen","zheng","zhi","zhong","zhou","zhu","zhua","zhuai","zhuan",
		"zhuang","zhui","zhun","zhuo","zi","zong","zou","zu","zuan","zui","zun","zuo"};

char mohu_shenmu[6][2][3]={{"z", "zh"},{"c", "ch"},{"s", "sh"},{"l", "n"},{"f", "h"},{"r", "l"}};
char mohu_yunmu[5][2][4]={{"an", "ang"},{"en", "eng"},{"in", "ing"}};
char mohu_yunmu2[2][2][5] ={{"ian", "iang"},{"uan","uang"}};

int py_lens[PINYIN_CLASS_LEN] ;
int hz_lens[PINYIN_CLASS_LEN] ;
pUTF8 hzs[PINYIN_CLASS_LEN];
int* hzs_pre_bndm[PINYIN_CLASS_LEN];

#define BUF_LEN_0  2048
BOOL load_hz(){
	FILE *fp;
	char buf[BUF_LEN_0];
	int line_count=0;
	fp = fopen("hz.lib", "r");
	if(fp==NULL) return 0;
	while(fgets(buf,BUF_LEN_0,fp)!=NULL){
//Reads  at  most  N-1  characters  from FP until a newline is found.
//The characters including to the newline are stored in BUF.  The  buffer  is terminated with a 0.
		int len = (int)strlen(buf);
		int last = len-1;
		pUTF8 str = (pUTF8)malloc_safe(len);
		strncpy(str,buf,len);
		str[last]='\0';
		if(len%3 == 2){
			str[last-1]='\0';
			hz_lens[line_count] = len/3;
		}else if(len%3 == 0){
			str[last-1]='\0';
			str[last-2]='\0';
			hz_lens[line_count] = len/3 -1;
		}else{
			hz_lens[line_count] = len/3;
		}
		hzs[line_count] = str;
		//printf("%d,%d,%d,%s\n",line_count,len,hz_lens[line_count],pinyins[line_count]);
		if((line_count++)>=PINYIN_CLASS_LEN) break;
	}
	fclose(fp);
	if(line_count==PINYIN_CLASS_LEN) return 1;
	return 0;
}

void print_hz_len(pUTF8 hz, int len){
	char chars[BUF_LEN_0] = {0};
	int d;
	WCHAR *wstr = utf8_to_wchar(hz,len,&d);
	int flen = WideCharToMultiByte(CP_OEMCP,(DWORD) 0,wstr,d,chars,BUF_LEN_0,NULL,FALSE);
	chars[flen]='\0';
	free_safe(wstr);
	//printf("%d,",d,chars);
	printf("%d,%s \n",d,chars);
}

void print_hz(pUTF8 hz){
	print_hz_len(hz,(int)strlen(hz));
}

void print_hzs(){
	int i=0;
	for(;i<PINYIN_CLASS_LEN;i++){
		print_hz(hzs[i]);
	}
}

void pre_bndm_cn(int index){
	int i;
	int count = hz_lens[index]/10;
	if(hz_lens[index]%10!=0) count+=1;
	hzs_pre_bndm[index] = (int *)malloc_safe(count*sizeof(int)*ASIZE);
	memset(hzs_pre_bndm[index], 0, count*sizeof(int)*ASIZE);
	for(i=0;i<count;i++){
		pUTF8 hz = hzs[index]+i*10*3;
		if(i<count-1){
			pre_bndm_hz_multi(hz,10*3,hzs_pre_bndm[index]+i*ASIZE);
		}else{
			pre_bndm_hz_multi(hz,(hz_lens[index]%10)*3,hzs_pre_bndm[index]+i*ASIZE);
		}
	}
}

int hz_match_one(int index, pUTF8 str, int strlen){
	int i;
	int count = hz_lens[index]/10;
	if(hz_lens[index]%10!=0) count+=1;
	for(i=0;i<count;i++){
		pUTF8 hz = hzs[index]+i*10*3;
		int match=0;
		if(i<count-1){
			match = bndm_hz_multi(hz,10*3,str,strlen,hzs_pre_bndm[index]+i*ASIZE);
			if(match) return match;
		}else{
			match = bndm_hz_multi(hz,(hz_lens[index]%10)*3,str,strlen,hzs_pre_bndm[index]+i*ASIZE);
			if(match) return match;
		}
	}
	return 0;
}

void pre_bndm_all(){//会一次性消耗3M内存
	int i=0;
	for(;i<PINYIN_CLASS_LEN;i++){
		pre_bndm_cn(i);
	}
}

BOOL init_chinese(){
	int i=0;
	for(i=0;i<PINYIN_CLASS_LEN;i++){
		py_lens[i] = (int)strlen(pinyins[i]);
	}
	load_hz();
	return 1;
}

