#include "env.h"
#include <stdio.h>
#include <time.h>
#include "global.h"
#include "fs_common.h"
#include "suffix.h"
#include "ntfs.h"
#include "fat.h"
#include "macfs.h"

const int ROOT_NUMBER=5;

BOOL attachParent(pFileEntry file, int i){
	pFileEntry parent;
	if(file==g_rootVols[i]) return 0;
	parent = findDir(file->up.ParentFileReferenceNumber,i);
	if(parent==NULL){
		FERROR(file);
		return 0;
	}
	addChildren(parent,file);
	return 1;
}

INLINE static BOOL match(pFileEntry file, void *data){
	return file->FileReferenceNumber == *(KEY *)data;
}

pFileEntry findFile(KEY frn,KEY pfrn,int i){
	pFileEntry parent = findDir(pfrn,i);
	if(parent==NULL) return NULL;
	return SubDirIterateB(parent,match,(void *)&frn);
}

void set_root_name_offline(pFileEntry offline_drive, int load_index){
    char root_name[3];
    _itoa(load_index,root_name,10);
    memcpy(offline_drive->FileName,root_name,2);
}


pFileEntry genRootFileEntry(int drive){
	NEW0(FileEntry, ret);
	ret->FileReferenceNumber = ROOT_NUMBER;
	ret->up.ParentFileReferenceNumber = 0;
	SET_ROOT_NAME(ret,drive);
    ret->us.v.FileNameLength = strlen(ret->FileName);
    ret->us.v.StrLen = ret->us.v.FileNameLength;
	ret->us.v.dir = 1;
	ret->ut.v.suffixType = SF_DIR;
	ret->up.parent = NULL;
	ret->children = NULL;
	g_rootVols[drive] = ret;
	ALL_FILE_COUNT +=1;
	return ret;
}

MINUTE GET_TIME(pFileEntry file){
	return (file->ut.v.time1 << 16) | (file->ut.v.time2 << 8) | file->ut.v.time3;
}

void SET_TIME(pFileEntry file, MINUTE time){
	file->ut.v.time1 = (time) >> 16 ;
	file->ut.v.time2 = (time >> 8) & 0xFF ;
	file->ut.v.time3 = time & 0xFF;
}

FSIZE GET_SIZE(pFileEntry file){
	return (file->us.v.size1<<8) | file->us.v.size2;
}

void SET_SIZE(pFileEntry file, FSIZE size){
	file->us.v.size1 = (size) >> 8 ;
	file->us.v.size2 = (size) & 0xFF ;
}

void print_full_path(pFileEntry pf){
	if(pf->up.parent!=NULL){
		print_full_path(pf->up.parent);
#ifdef WIN32
		printf("\\");
#else
        printf("/");
#endif
	}
	printf("%s",pf->FileName);
}

int print_fullpath_str(pFileEntry file, char *p){
	char *buffer = p;
	if(file->up.parent!=NULL){
		int size = print_fullpath_str(file->up.parent,buffer);
		buffer += size;
	}
	memcpy(buffer,file->FileName,file->us.v.FileNameLength);
	buffer += file->us.v.FileNameLength;
#ifdef WIN32
	*buffer++ ='\\';
	*buffer++ ='\\';
#else
    *buffer++ ='/';
#endif
	return buffer-p;
}

int print_path_str(pFileEntry file, char *p){
	char *buffer = p;
	if(file->up.parent!=NULL){
		return print_fullpath_str(file->up.parent,buffer);
	}
	return 0;
}

//年5b    月4b    日5b     时5b   分5b
//00000   0000   00000  00000    00000
#ifndef YEAR_START
#define YEAR_START 1990
#endif

#ifdef WIN32
INLINE MINUTE ConvertSystemTimeToMinute(SYSTEMTIME sysTime)
{
    MINUTE time32=0;
	if(sysTime.wYear<YEAR_START) return 0;
    time32=sysTime.wMinute/2;
    time32|=(sysTime.wHour<<5);
    time32|=(sysTime.wDay<<10);
    time32|=(((DWORD)sysTime.wMonth)<<15);
    time32|=((DWORD)(sysTime.wYear-YEAR_START)<<19);
    return time32;
}

INLINE void ConvertMinuteToSystemTime(SYSTEMTIME *sysTime,IN MINUTE time32)
{
    sysTime->wMinute=(time32&0x1f)*2;time32>>=5;
    sysTime->wHour=time32&0x1f;time32>>=5;
    sysTime->wDay=time32&0x1f;time32>>=5;
    sysTime->wMonth=time32&0xf;time32>>=4;
    sysTime->wYear=time32+YEAR_START;
}

void set_time(pFileEntry file, PFILETIME time){
	SYSTEMTIME st;
	FILETIME fileTime;
	FileTimeToLocalFileTime(time,&fileTime);
	FileTimeToSystemTime(&fileTime,&st);
	SET_TIME(file,ConvertSystemTimeToMinute(st));
}
#else
INLINE MINUTE ConvertSystemTimeToMinute(time_t sysTime)
{
    MINUTE time32=0;
	struct tm *st = localtime(&sysTime);
	if(st->tm_year < YEAR_START) return 0;
    time32=st->tm_min/2;
    time32|=(st->tm_hour << 5 );
    time32|=(st->tm_mday << 10);
    time32|=(((DWORD)st->tm_mon)<<15);
    time32|=((DWORD)(st->tm_year-YEAR_START)<<19);
    return time32;
}

INLINE void ConvertMinuteToSystemTime(struct tm *st, MINUTE time32)
{
    st->tm_min=(time32&0x1f)*2;time32>>=5;
    st->tm_hour=time32&0x1f;time32>>=5;
    st->tm_mday=time32&0x1f;time32>>=5;
    st->tm_mon=time32&0xf;time32>>=4;
    st->tm_year=time32+YEAR_START;
}

void set_time(pFileEntry file, time_t time){
	SET_TIME(file,ConvertSystemTimeToMinute(time));
}
#endif //WIN32

void print_time(pFileEntry file){
	char buf[256];
	print_time_str(file,buf);
	printf("%s",buf);
}

int print_time_str(pFileEntry file, char *buffer){
#ifdef WIN32
	WCHAR buf[128];
	int ret,wret;
	SYSTEMTIME st;
	ConvertMinuteToSystemTime(&st,GET_TIME(file));
	wret = swprintf(buf,128,L"%d年%d月%d日 %d时%d分"
		,st.wYear,st.wMonth,st.wDay
		,st.wHour,st.wMinute
		);
	ret = wchar_to_utf8_len(buf, wret);
	wchar_to_utf8_nocheck(buf,wret,buffer,ret);
	return ret;
#else
	struct tm st;
	ConvertMinuteToSystemTime(&st,GET_TIME(file));
	return sprintf(buffer,"%d年%d月%d日 %d时%d分"
		,st.tm_year,st.tm_mon,st.tm_mday
		,st.tm_hour,st.tm_min
		);	
#endif
}


#ifdef WIN32
BOOL StartMonitorThread(int i){
	if(is_cdrom_drive(i)) return 0;
	if(IsNtfs(i)){
		return StartMonitorThreadNTFS(i);
	}else{
		OpenFatHandle(i);
		return StartMonitorThreadFAT(i);
	}
}

BOOL StopMonitorThread(int i){
	if(g_hThread[i]){
		return TerminateThread(g_hThread[i],0);
	}
	return 0;
}
#else
BOOL StartMonitorThread(int i){
	return StartMonitorThreadMAC(i);
}

BOOL StopMonitorThread(int i){
	return StopMonitorThreadMAC(i);
}

#endif //WIN32


#ifdef WIN32
void FileRemoveFilter(pFileEntry file, void *data){
    //TODO: ‘$文件’更改为系统文件类型，而不是删除
	if(IsDir(file)){
		if(file->FileName[0]=='$'
           || (file->us.v.FileNameLength==4 && file->FileName[0]=='.' && file->FileName[1]=='s' && file->FileName[2]=='v'  && file->FileName[3]=='n')
           || (file->us.v.FileNameLength==4 && file->FileName[0]=='.' && file->FileName[1]=='g' && file->FileName[2]=='i'  && file->FileName[3]=='t')
           || (file->us.v.FileNameLength==3 && file->FileName[0]=='C' && file->FileName[1]=='V' && file->FileName[2]=='S')
           ){
            file->children=NULL;
		}
		if((file->us.v.FileNameLength==4 && strncmp(file->FileName,"Temp",4)==0)
           || (file->us.v.FileNameLength==24 && strncmp(file->FileName,"Temporary Internet Files",24)==0)
           ){
			if((file->up.parent->us.v.FileNameLength==14 && strncmp(file->up.parent->FileName,"Local Settings",14)==0)
               ){
                file->children=NULL;
			}
		}
		if(file->us.v.FileNameLength==5 && strncmp(file->FileName,"cache",5)==0){
			if(file->up.parent->us.v.FileNameLength==3 && strncmp(file->up.parent->FileName,"var",3)==0){
                file->children=NULL;
			}
		}
	}
}
#else
void FileRemoveFilter(pFileEntry file, void *data){
    if(IsDir(file)){
        if(file->us.v.FileNameLength==7 && strncmp(file->FileName,"private",7)==0){
			if(IsRoot(file->up.parent->FileReferenceNumber)) file->children=NULL;
		}
    }
}
#endif //WIN32




BOOL check_file_entry(pFileEntry file, void *data){
	if(file==NULL) return 1;
	if(IsDir(file)){
		if( file->ut.v.suffixType != SF_DIR) return 0;
	}else{
		if( file->children!=NULL) return 0;
		if( file->up.parent==NULL) return 0;
	}
	return 1;
}

