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
			MATCH_SUFFIX(VEDIO,len);			\
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
static char PHOTO3[][4] = { "jpg","jpe","git","png","bmp","dib","tif","pcx","ico","pcd","psd"};
static char PHOTO4[][5] = { "jpeg","jeif","tiff"};

static char VEDIO1[][2] = { 0};
static char VEDIO2[][3] = { 0};
static char VEDIO3[][4] = { "avi","asf","wmv","avs","flv","mkv","mov","3gp","mp4","mpg","ogm","vob","rm","ts","tp","ifo","nsv"};
static char VEDIO4[][5] = { "mpeg","rmvb"};

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

__forceinline unsigned char suffix_type(pUTF8 suffix, int len){
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
