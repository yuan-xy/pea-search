#include <stdio.h>
#include <ctype.h>
#include "suffix.h"
#include "fs_common.h"


#define NELEMS(x) ((sizeof (x))/(sizeof ((x)[0])))

#define MATCH_SUFFIX(type,len) for(i=0;i<NELEMS(type##len);i++) if(strncmp(copy,type##len[i],3)==0) return SF_##type

#define MATCH_SUFFIX_ALL(len)				 \
			MATCH_SUFFIX(HTM,len);			\
			MATCH_SUFFIX(SOURCE,len);			\
			MATCH_SUFFIX(EXE,len);			\
			MATCH_SUFFIX(SCRIPT,len);			\
			MATCH_SUFFIX(LIB,len);			\
			MATCH_SUFFIX(MUSIC,len);			\
			MATCH_SUFFIX(PHOTO,len);			\
			MATCH_SUFFIX(VIDEO,len);			\
			MATCH_SUFFIX(ANIMATION,len);			\
			MATCH_SUFFIX(WORD,len);			\
			MATCH_SUFFIX(EXCEL,len);			\
			MATCH_SUFFIX(PPT,len);			\
			MATCH_SUFFIX(OTHER_OFFICE,len);			\
			MATCH_SUFFIX(OTHER_EBOOK,len);			\
			MATCH_SUFFIX(DISK,len);			\
			MATCH_SUFFIX(OTHER_ZIP,len);			

static char DISK1[][2] = { 0};
static char DISK2[][3] = { 0};
static char DISK3[][4] = { "iso","cue","img","cif","nrg","fcd","vcd","mds"};
static char DISK4[][5] = { 0};

static char OTHER_ZIP1[][2] = { "z"};
static char OTHER_ZIP2[][3] = { "gz"};
static char OTHER_ZIP3[][4] = { "cab","arj","lzh","ace","tar","bz2"};
static char OTHER_ZIP4[][5] = { 0};

static char EXE1[][2] = { 0};
static char EXE2[][3] = { 0};
static char EXE3[][4] = { "exe","com","cpl","scr"};
static char EXE4[][5] = { 0};

static char SCRIPT1[][2] = { 0};
static char SCRIPT2[][3] = { 0};
static char SCRIPT3[][4] = { "js","vbs","cmd","bat","reg","inf"};
static char SCRIPT4[][5] = { 0};

static char LIB1[][2] = { 0};
static char LIB2[][3] = { 0};
static char LIB3[][4] = { "dll","jar","lib"};
static char LIB4[][5] = { 0};

static char MUSIC1[][2] = { 0};
static char MUSIC2[][3] = { "au","ra"};
static char MUSIC3[][4] = { "aac","ac3","aif","snd","cda","dts","mid","mod","mp3","mp2","mp1","mpa","ape","mpc","mp+","tta","ogg","wav","wma","m3u"};
static char MUSIC4[][5] = { "flac","midi","ttpl","ttbl","aifc","aiff"};

static char PHOTO1[][2] = { 0};
static char PHOTO2[][3] = { 0};
static char PHOTO3[][4] = { "jpg","jpe","gif","png","bmp","dib","tif","pcx","ico","pcd","psd"};
static char PHOTO4[][5] = { "jpeg","jeif","tiff"};

static char VIDEO1[][2] = { 0};
static char VIDEO2[][3] = { 0};
static char VIDEO3[][4] = { "avi","asf","wmv","avs","flv","mkv","mov","3gp","mp4","mpg","ogm","vob","rm","ts","tp","ifo","nsv"};
static char VIDEO4[][5] = { "mpeg","rmvb"};

static char ANIMATION1[][2] = { 0};
static char ANIMATION2[][3] = { 0};
static char ANIMATION3[][4] = { "swf","svg","fla","flc","pcs","pic"};
static char ANIMATION4[][5] = { 0};

static char WORD1[][2] = { 0};
static char WORD2[][3] = { 0};
static char WORD3[][4] = { "doc","dot","rtf"};
static char WORD4[][5] = { "docx","docm","dotx"};

static char EXCEL1[][2] = { 0};
static char EXCEL2[][3] = { 0};
static char EXCEL3[][4] = { "xls","xlt","csv"};
static char EXCEL4[][5] = { "xlsx","xlsm","xlsb","xlam","xltx","xltm"};

static char PPT1[][2] = { 0};
static char PPT2[][3] = { 0};
static char PPT3[][4] = { "ppt","pps","pot"};
static char PPT4[][5] = { "pptx","pptm","ppsx","ppsm","potx","potm"};

static char OTHER_OFFICE1[][2] = { 0};
static char OTHER_OFFICE2[][3] = { 0};
static char OTHER_OFFICE3[][4] = { "mdb","adp","grv","xml","eml","one","pub","vsd","vdx","vss","vsx","vst","odt","ods","odp","odg","odm","ots","otp","ott","123","wkl","wks","sda","sdd"};
static char OTHER_OFFICE4[][5] = { 0};

static char OTHER_EBOOK1[][2] = { 0};
static char OTHER_EBOOK2[][3] = { 0};
static char OTHER_EBOOK3[][4] = { "pdg","caj","djv"};
static char OTHER_EBOOK4[][5] = { "djvu"};

static char HTM1[][2] = { 0};
static char HTM2[][3] = { 0};
static char HTM3[][4] = { "htm","mht","xml"};
static char HTM4[][5] = { "html"};

static char SOURCE1[][2] = { "c","h","s","d"};
static char SOURCE2[][3] = { "cc","as","cs","vb","rb","py"};
static char SOURCE3[][4] = { "cpp","hpp","pas","php","css","asp","jsp"};
static char SOURCE4[][5] = { "java","yaml","aspx"};

static char OTHER_TEXT1[][2] = { 0};
static char OTHER_TEXT2[][3] = { 0};
static char OTHER_TEXT3[][4] = { "log","ini","lrc"};
static char OTHER_TEXT4[][5] = { 0};

unsigned char suffix_type0(char copy[], int len){
	int i;
	if(len==3){
		if(strncmp(copy,"zip",3)==0) return SF_ZIP;
		if(strncmp(copy,"rar",3)==0) return SF_RAR;
		if(strncmp(copy,"lnk",3)==0) return SF_LNK;
		if(strncmp(copy,"pdf",3)==0) return SF_PDF;
		if(strncmp(copy,"chm",3)==0) return SF_CHM;
		if(strncmp(copy,"txt",3)==0) return SF_TXT;
		//for(j=0;j<NELEMS(DISK3);j++) if(strncmp(copy,DISK3[j],3)==0) return SF_DISK;
		MATCH_SUFFIX_ALL(3);
	}else if(len==4){
		MATCH_SUFFIX_ALL(4);
	}else if(len==2){
		MATCH_SUFFIX_ALL(2);
	}else if(len==1){
		MATCH_SUFFIX_ALL(1);
	}else if(len==5){
		if(strncmp(copy,"shtml",5)==0) return SF_HTM;
		if(strncmp(copy,"mhtml",5)==0) return SF_HTM;
		if(strncmp(copy,"accdb",5)==0) return SF_OTHER_OFFICE;
		if(strncmp(copy,"7-zip",5)==0) return SF_OTHER_ZIP;
	}else{
		if(strncmp(copy,"mp3pro",6)==0) return SF_MUSIC;
	}
	return SF_UNKNOWN;
}

static char last_str[7];
static unsigned char last_type;

INLINE unsigned char suffix_type(pUTF8 suffix, int len){
	char copy[7];
	int i=0;
	if(len==0) return SF_NONE;
	if(len>6) return SF_UNKNOWN;
	for(;i<len;i++) copy[i] = tolower(*(suffix+i));
	copy[len]='\0';
	if(strcmp(last_str,copy)==0) return last_type;
	strcpy(last_str,copy);
	last_type = suffix_type0(copy,len);
	return last_type;
}

BOOL include_type(unsigned char clazz, unsigned char file_type){
	switch(clazz){
		case SFV_ARCHIVE: return IS_ARCHIVE(file_type);
		case SFV_COMPRESS: return IS_COMPRESS(file_type);
		case SFV_EBOOK: return IS_EBOOK(file_type);
		case SFV_OFFICE: return IS_OFFICE(file_type);
		case SFV_PROGRAM: return IS_PROGRAM(file_type);
		case SFV_TEXT: return IS_TEXT(file_type);
		case SFV_MEDIA: return IS_MEDIA(file_type);
		case SFV_OTHER: return IS_OTHER(file_type);
		default:		return clazz==file_type;
	}
}

void SuffixProcess(pFileEntry file, void *data){
	int len=file->us.v.FileNameLength;
	int j=len-2;
	int OnlyNameLen = 0;
	if(file->FileName[len-1]=='.'){
		OnlyNameLen = len-1;
			goto find_dot;
	}
	for(;j>=0;j--){
		if(file->FileName[j]=='.'){
			OnlyNameLen = j;
			goto find_dot;
		}
	}
	OnlyNameLen = file->us.v.FileNameLength;
find_dot:
	if(IsDir(file)){
		file->ut.v.suffixType = SF_DIR;
	}else if(OnlyNameLen == file->us.v.FileNameLength || OnlyNameLen == file->us.v.FileNameLength-1 ){
		file->ut.v.suffixType = SF_NONE;
	}else{
		file->ut.v.suffixType = suffix_type(file->FileName+OnlyNameLen+1,len-OnlyNameLen-1);
	}
}


int print_suffix_type(pFileEntry file, char *buffer){
	unsigned char clazz = file->ut.v.suffixType;
	switch(clazz){
		case SF_NONE: 			strcpy(buffer,"none");return 4;
		case SF_UNKNOWN: 		strcpy(buffer,"unknown");return 7;
		case SF_DIR	: 		strcpy(buffer,"dir");return 3;
		case SF_DISK	: 		strcpy(buffer,"disk");return 4;
		case SF_ZIP	: 		strcpy(buffer,"zip");return 3;
		case SF_RAR	: 		strcpy(buffer,"rar");return 3;
		case SF_OTHER_ZIP: 		strcpy(buffer,"other_zip");return 9;
		case SF_EXE	: 		strcpy(buffer,"exe");return 3;
		case SF_LNK	: 		strcpy(buffer,"link");return 4;
		case SF_SCRIPT: 		strcpy(buffer,"script");return 6;
		case SF_LIB	: 		strcpy(buffer,"lib");return 3;
		case SF_MUSIC: 			strcpy(buffer,"music");return 5;
		case SF_PHOTO: 			strcpy(buffer,"photo");return 5;
		case SF_VIDEO: 			strcpy(buffer,"video");return 5;
		case SF_ANIMATION: 		strcpy(buffer,"animation");return 9;
		case SF_WORD	: 		strcpy(buffer,"word");return 4;
		case SF_EXCEL: 			strcpy(buffer,"excel");return 5;
		case SF_PPT	: 		strcpy(buffer,"ppt");return 3;
		case SF_OTHER_OFFICE: 		strcpy(buffer,"other_office");return 12;
		case SF_PDF	: 		strcpy(buffer,"pdf");return 3;
		case SF_CHM	: 		strcpy(buffer,"chm");return 3;
		case SF_OTHER_EBOOK: 		strcpy(buffer,"other_ebook");return 11;
		case SF_HTM	: 		strcpy(buffer,"htm");return 3;
		case SF_TXT	: 		strcpy(buffer,"txt");return 3;
		case SF_SOURCE : 		strcpy(buffer,"source");return 6;
		case SF_OTHER_TEXT : 		strcpy(buffer,"other_text");return 10;
		default:		strcpy(buffer,"unknown");return 7;
	}
	return 0;
}

int print_suffix_type2(int index, char *buffer){
	switch(index){
		case 0: 	strcpy(buffer,"none");return 4;
		case 1: 	strcpy(buffer,"unknown");return 7;
		case 2: 	strcpy(buffer,"dir");return 3;
		case 3: 	strcpy(buffer,"disk");return 4;
		case 4: 	strcpy(buffer,"zip");return 3;
		case 5: 	strcpy(buffer,"rar");return 3;
		case 6: 	strcpy(buffer,"other_zip");return 9;
		case 7: 	strcpy(buffer,"exe");return 3;
		case 8: 	strcpy(buffer,"link");return 4;
		case 9: 	strcpy(buffer,"script");return 6;
		case 10:	strcpy(buffer,"lib");return 3;
		case 11:	strcpy(buffer,"music");return 5;
		case 12:	strcpy(buffer,"photo");return 5;
		case 13:	strcpy(buffer,"video");return 5;
		case 14:	strcpy(buffer,"animation");return 9;
		case 15:	strcpy(buffer,"word");return 4;
		case 16:	strcpy(buffer,"excel");return 5;
		case 17:	strcpy(buffer,"ppt");return 3;
		case 18:	strcpy(buffer,"other_office");return 12;
		case 19:	strcpy(buffer,"pdf");return 3;
		case 20:	strcpy(buffer,"chm");return 3;
		case 21:	strcpy(buffer,"other_ebook");return 11;
		case 22:	strcpy(buffer,"htm");return 3;
		case 23:	strcpy(buffer,"txt");return 3;
		case 24:	strcpy(buffer,"source");return 6;
		case 25:	strcpy(buffer,"other_text");return 10;
		default:		strcpy(buffer,"unknown");return 7;
	}
	return 0;
}
