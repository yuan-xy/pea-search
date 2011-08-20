#define _CRT_SECURE_NO_DEPRECATE 

#include "env.h"
#include <stdio.h>
#include <time.h>
#include "drive.h"
#include "write.h"
#include "global.h"
#include "util.h"
#include "search.h"
#include "fs_common.h"
#include "bzip2/bzlib.h"
#include "ntfs.h"
#include "main.h"

static unsigned short MAGIC = 0x1234;
static short MAGIC_LEN = sizeof(MAGIC);
static int ZERO=0;

__declspec (thread) static BOOL is_ntfs_cur_drive=1;

#ifdef USE_ZIP
	static char file_name_pattern[] = "%x.dbz";
#else
	static char file_name_pattern[] = "%x.db";
#endif

#define FILE_NAME(i)		\
		char name[32];		\
		sprintf(name,file_name_pattern,g_VolsInfo[i].serialNumber);

void FileWriteVisitor(pFileEntry file, void *data){
	FILE *fp = (FILE *)data;
	if(is_ntfs_cur_drive){
		fwrite(&file->FileReferenceNumber,sizeof(KEY),1,fp);
		if(file->up.parent==NULL){
			fwrite(&ZERO,sizeof(KEY),1,fp);
		}else{
			fwrite(&file->up.parent->FileReferenceNumber,sizeof(KEY),1,fp);
		}
	}else{
		fwrite(&file,sizeof(KEY),1,fp);
		if(file->up.parent==NULL){
			fwrite(&ZERO,sizeof(KEY),1,fp);
		}else{
			fwrite(&file->up.parent,sizeof(KEY),1,fp);
		}
	}
	{
		char *p = (char *) file;
		fwrite(p+WORD_SIZE*3,FILE_ENTRY_SIZE(file)-WORD_SIZE*3,1,fp);
	}
}

BOOL save2file0(int i){
	FILE *fp;
	FILE_NAME(i);
	fp = fopen(name, "wb");
	if(fp==NULL) return 0;
	fwrite(&MAGIC,MAGIC_LEN,1,fp);
	fwrite(&FILE_SEARCH_MAJOR_VERSION,1,1,fp);
	fwrite(&FILE_SEARCH_MINOR_VERSION,1,1,fp);
	fwrite(&(g_VolsInfo[i]),sizeof(g_VolsInfo[i]),1,fp);
	is_ntfs_cur_drive = IsNtfs(i);
	if(is_ntfs_cur_drive){
		fwrite(&(g_curFirstUSN[i]),sizeof(g_curFirstUSN[i]),1,fp);
		fwrite(&(g_curNextUSN[i]),sizeof(g_curNextUSN[i]),1,fp);
		fwrite(&(g_curJournalID[i]),sizeof(g_curJournalID[i]),1,fp);
	}else{
		time_t start = time(NULL);
		fwrite(&start,sizeof(time_t),1,fp);
	}
	FilesIterate(g_rootVols[i],FileWriteVisitor,(void *)fp);
	fclose(fp);
	return 1;
}

BOOL save2filezip(int i){
	return 1;
}

BOOL save_db(int i){
	if(!g_loaded[i]) return 1;
	#ifdef USE_ZIP
		return save2filezip(i);
	#else
		return save2file0(i);
	#endif
}

void save_db_all(){
	ValidDrivesIterator(save_db);
}

BOOL readfile(int i, char *filename){
	BOOL gen_root=0;
	int count=0,d=0;
	FILE *fp;
	if(filename!=NULL){
		fp = fopen(filename, "rb");
	}else{
		FILE_NAME(i);
		fp = fopen(name, "rb");
	}
	if(fp==NULL) return 0;
	{
		unsigned short magic;
		unsigned short major_ver;
		unsigned short minor_ver;
		DriveInfo info;
		fread(&magic,MAGIC_LEN,1,fp);
		if(magic!=MAGIC) goto error;
		d=(int)fread(&major_ver,1,1,fp);
		if(d<1) goto error;
		d=(int)fread(&minor_ver,1,1,fp);
		if(d<1) goto error;
		fread(&info,sizeof(DriveInfo),1,fp);
		if(i<DIRVE_COUNT){
			if(info.serialNumber!=g_VolsInfo[i].serialNumber) goto error;
			if(IsNtfs(i)){
				USN         first_usn;
				USN         next_usn;
				DWORDLONG   jid;
				d=(int)fread(&first_usn,sizeof(USN),1,fp);
				if(d<1) goto error;
				d=(int)fread(&next_usn,sizeof(USN),1,fp);
				if(d<1) goto error;
				d=(int)fread(&jid,sizeof(DWORDLONG),1,fp);
				if(d<1) goto error;
                if(jid!=g_curJournalID[i]) g_expires[i]=1;
                my_assert(next_usn<=g_curNextUSN[i],0);
                if(next_usn < g_curFirstUSN[i]) g_expires[i]=1;
			}else{
				time_t now = time(NULL);
				time_t last;
				d=(int)fread(&last,sizeof(time_t),1,fp);
				if(d<1) goto error;
				if(now-last>3600*8)  g_expires[i]=1;
			}
		}else{
			g_VolsInfo[i] = info;
			if(IsNtfs(i)){
				USN         first_usn;
				USN         next_usn;
				DWORDLONG   jid;
				d=(int)fread(&first_usn,sizeof(USN),1,fp);
				if(d<1) goto error;
				d=(int)fread(&next_usn,sizeof(USN),1,fp);
				if(d<1) goto error;
				d=(int)fread(&jid,sizeof(DWORDLONG),1,fp);
				if(d<1) goto error;
			}else{
				time_t last;
				d=(int)fread(&last,sizeof(time_t),1,fp);
				if(d<1) goto error;
			}
		}
	}
	do{
		NEW0(FileEntry,file);
		d = (int)fread(file,WORD_SIZE,2,fp);
		if(d<2){
				if(feof(fp)) goto ok;
				else goto error;
		}
		d = (int)fread(&file->us.value,sizeof(file->us),2,fp);
		if(d<2) goto error;
		if(file->us.v.FileNameLength == 0){
			continue;
		}
		file = (pFileEntry) realloc_safe(file,FILE_ENTRY_SIZE(file));
		d = (int)fread(file->FileName,sizeof(char),file->us.v.FileNameLength,fp);
		if(d<file->us.v.FileNameLength) goto error;
		file->children = NULL;
		add2Map(file,i);
		if(!gen_root){
			if(i<26){
				if(strncmp(file->FileName,rootNames[i],2)!=0) SET_ROOT_NAME(file,rootNames[i]);
				if(IsNtfs(i)){
					if(!IsRoot(file->FileReferenceNumber)) goto error;
				}
			}else{
				char root_name[3];
				_itoa(i,root_name,10);
				SET_ROOT_NAME(file,root_name); //离线文件的根目录
			}
			if(file->up.ParentFileReferenceNumber!=0) goto error;
			g_rootVols[i] = file;
			gen_root=1;
		}
		ALL_FILE_COUNT +=1;
		count++;
	}while(1);
ok:
#ifdef MY_DEBUG
	printf("load %d from file for drive %c\n",count, i<26? i+'A':i );
#endif
	fclose(fp);
	return 1;
error:
	fclose(fp);
	return 0;
}

BOOL readfileZip(int i, char *filename){
	return 0;
}

BOOL load_db(int i){
	#ifdef USE_ZIP
		return readfileZip(i,NULL);
	#else
		return readfile(i,NULL);
	#endif
}

void del_offline_db(int i){
	char cmd[64];
	FILE_NAME(i);
	g_loaded[i] = 0;
	deleteDir(g_rootVols[i]);
	sprintf(cmd,"del %s",name);
	system(cmd);
}

static int load_offline_db_one(int i,char *file){
	#ifdef USE_ZIP
		return readfileZip(i,file);
	#else
		return readfile(i,file);
	#endif
}

static BOOL loaded(char *filename){
	int i=0;
	for(;i<DIRVE_COUNT_OFFLINE;i++){
		if(g_loaded[i]){
			FILE_NAME(i);
			if(stricmp(filename,name)==0) return 1;
		}
	}
	return 0;
}

void DbIterator(pDbVisitor visitor, void *data){
	WIN32_FIND_DATAA fd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	hFind = FindFirstFileA(".\\*.db", &fd);
	if (INVALID_HANDLE_VALUE == hFind){
		FindClose(hFind);
		return;
	}
	do{
		BOOL ret = (*visitor)(fd.cFileName,data);
		if(!ret) break;
	}while (FindNextFileA(hFind, &fd) != 0);
	FindClose(hFind);
}

BOOL offline_db_visitor(char *db_name, void *data){
		int *pi = (int *)data;
		BOOL flag;
		if(loaded(db_name)) return 1;
		flag = load_offline_db_one(*pi,db_name);
		if(flag){
			build_dir(*pi);
			after_build(*pi);
			g_loaded[*pi]=1;
			(*pi)++;
		}
		if((*pi)>=DIRVE_COUNT_OFFLINE) return 0;
		return 1;
}

int load_offline_dbs(){
	int i=DIRVE_COUNT;
	DbIterator(offline_db_visitor,&i);
	return i-DIRVE_COUNT;
}

#undef _CRT_SECURE_NO_DEPRECATE 
