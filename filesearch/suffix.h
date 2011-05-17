#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_SUFFIX_H_
#define FILE_SEARCH_SUFFIX_H_

#include <windows.h>
#include "util.h"
#include "fs_common.h"

/*
0	none
1	unknown
10	dir
0000			01		00   disk image  //iso cue img cif nrg fcd vcd mds
				10	zip	01 zip
						10 rar
						11 other_zip	//cab arj lzh ace 7-zip tar gzip uue bz2 z gz 
0001 program			0001 exe        //exe com cpl scr
						0010 lnk
						0011 script    //js vbs cmd bat reg inf
						0100 lib		//dll jar lib

0011 multimedia			0001 music     //aac ac3 aif aifc aiff au snd cda dts dtswav flac midi mid mod mp3 mp2 mp1 mpa mp3pro ape mpc mp+ ra tta ogg wav wma ttpl ttbl m3u 
						0010 photo     //jpg jpeg jpe jeif git png bmp dib tif tiff pcx ico pcd psd
						0011 video     //avi asf wmv avs flv mkv mov 3gp mp4 mpg mpeg ogm vob rm rmvb ts tp ifo nsv
						0100 animation  //swf fla flc pcs pic
011 archive	01office	001 word		//doc docx docm dotx dot rtf
						010 xls			//xls xlsx xlsm xlsb xlam xltx xltm xlt csv
						011 ppt			//ppt pptx pptm pps ppsx ppsm potx pot potm
						100 other_office //mdb accdb adp grv xml eml one pub vsd vdx vss vsx vst odt ods odp odg odm ots otp ott 123 wkl wks sda sdd
			10 ebook 
						001 pdf
						010 chm
						011 other_ebook //djvu pdg caj

						001 htm		//htm html shtml mhtml mht xml
			11 text
						010 txt
						011 source code // c cpp cc h hpp java cs vb pas rb py php css yaml s d  asp aspx jsp as 
						111 other_text //log ini lrc

*/
#define SF_NONE		0
#define SF_UNKNOWN		1
#define SF_DIR		BYTE_BIN(00000010)
#define SF_DISK		BYTE_BIN(00000100)


#define SF_ZIP		BYTE_BIN(00001001)
#define SF_RAR		BYTE_BIN(00001010)
#define SF_OTHER_ZIP BYTE_BIN(00001011)

#define SF_EXE		BYTE_BIN(00010001)
#define SF_LNK		BYTE_BIN(00010010)
#define SF_SCRIPT	BYTE_BIN(00010011)
#define SF_LIB		BYTE_BIN(00010100)

#define SF_MUSIC	BYTE_BIN(00110001)
#define SF_PHOTO	BYTE_BIN(00110010)
#define SF_VEDIO	BYTE_BIN(00110011)
#define SF_ANIMATION	BYTE_BIN(00110100)

#define SF_WORD		BYTE_BIN(01101001)
#define SF_EXCEL	BYTE_BIN(01101010)
#define SF_PPT		BYTE_BIN(01101011)
#define SF_OTHER_OFFICE BYTE_BIN(01101100)

#define SF_PDF		BYTE_BIN(01110001)
#define SF_CHM		BYTE_BIN(01110010)
#define SF_OTHER_EBOOK	BYTE_BIN(01110011)


#define SF_HTM		BYTE_BIN(01111001)
#define SF_TXT		BYTE_BIN(01111010)
#define SF_SOURCE		BYTE_BIN(01111011)
#define SF_OTHER_TEXT	BYTE_BIN(01111111)

#define SFV_COMPRESS BYTE_BIN(10000001)
#define SFV_PROGRAM	BYTE_BIN(10000010)
#define SFV_MEDIA	BYTE_BIN(10000011)
#define SFV_ARCHIVE	BYTE_BIN(10000100)
#define SFV_OFFICE	BYTE_BIN(10000101)
#define SFV_EBOOK	BYTE_BIN(10000110)
#define SFV_TEXT	BYTE_BIN(10000111)
#define SFV_OTHER	BYTE_BIN(10001111)

#define IS_OTHER(x) (x==0 || x==1)
#define IS_COMPRESS(x) (x >> 3)==1 || (x==SF_DISK)
#define IS_PROGRAM(x)  (x >> 4)==1
#define IS_MEDIA(x)		(x >> 4)==3
#define IS_ARCHIVE(x)		(x >> 5)==3
#define IS_OFFICE(x)		(x >> 3)==015
#define IS_EBOOK(x)			(x >> 3)==016
#define IS_TEXT(x)			(x >> 3)==017 && (x!=SF_HTM)

#define NON_VIRTUAL_TYPE_SIZE 26

/**
 * 根据后缀名得到文件的类型
 * @param suffix 后缀名
 * @len 后缀名的byte长度
 */
extern unsigned char suffix_type(pUTF8 suffix, int len);

/**
 * 后一个文件类型是否是前一个文件类型的子类
 * @param clazz 组合的或者单个的文件类型
 * @file_type 单个的文件类型
 */
extern BOOL include_type(unsigned char clazz, unsigned char file_type);

/**
 * 对文件进行后缀名处理
 * @param file 文件
 * @param data 无效参数，仅用于匹配FileVisitor的方法签名
 */
extern void SuffixProcess(pFileEntry file, void *data);

extern int print_suffix_type(pFileEntry file, char *p);
extern int print_suffix_type2(int index, char *buffer);

#endif  // FILE_SEARCH_SUFFIX_H_

#ifdef __cplusplus
}
#endif
