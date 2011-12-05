#include "env.h"
#include "global.h"
#include "fs_common.h"
#include "util.h"
#include "suffix.h"
#include "search.h"
#include <dirent.h>
#include <limits.h>
#include "unixfs.h"

BOOL same_file(pFileEntry file, struct dirent * dp){
    if(file==NULL || dp==NULL) return 0;
#ifdef APPLE
    if(dp->d_namlen != file->us.v.FileNameLength) return 0;
#endif
    return strcmp(dp->d_name, file->FileName)==0;
}

int ignore_filter_scandir(struct dirent *dp){
    return strcmp(dp->d_name, ".") != 0  && strcmp(dp->d_name, "..") != 0;
}

static BOOL dir_iterate_bool(char *dir_name, pDirentVisitorB visitor, va_list args, char *buffer, BOOL breakLoop){
    struct dirent * dp;
    DIR * dirp = opendir(dir_name);
    if(dirp==NULL) return 0;
    while ((dp = readdir(dirp)) != NULL){
        if (strcmp(dp->d_name, ".") == 0  || strcmp(dp->d_name, "..") == 0) continue;
        va_list ap2;
        va_copy(ap2,args);
        if(breakLoop){
            BOOL flag = (*visitor)(dir_name,dp,ap2);
            if(flag){
                if(buffer!=NULL) strcpy(buffer, dp->d_name);
                closedir(dirp); 
                return 1; 
            } 
        }else{
            (*visitor)(dir_name,dp,ap2);
        }
        va_end(ap2);
    }
    closedir(dirp); 
    return 0;
}

void dir_iterate(pDirentVisitor visitor, char *dir_name, ...){
    va_list args;
	va_start (args, dir_name);
    dir_iterate_bool(dir_name, (pDirentVisitorB)visitor, args, NULL, 0);
    va_end(args);
}

BOOL dir_iterateB(pDirentVisitorB visitor, char *dir_name, char *buffer, ...){
    BOOL ret;
    va_list args;
	va_start (args, buffer);
    ret = dir_iterate_bool(dir_name, visitor, args, buffer, 1);
    va_end(args);
    return ret;
}

pFileEntry initUnixFile(const struct stat *statptr, char *filename, pFileEntry parent){
	int len = strlen(filename);//TODO: 直接传递d_namlen
	NEW0_FILE(ret,len);
	ret->us.v.FileNameLength = len;
	//ret->us.v.StrLen = utf8_to_wchar_len(filename,len);
	strncpy(ret->FileName,filename,len);
	if(S_ISDIR(statptr->st_mode)) {
		ret->us.v.dir = 1;
        ret->ut.v.suffixType = SF_DIR;
	}
    SuffixProcess(ret,NULL);
    if(*(ret->FileName)=='.') ret->us.v.hidden = 1;
    if(!is_important_type(ret->ut.v.suffixType)){
        if(parent->us.v.hidden || *(ret->FileName)=='$' || *(ret->FileName)=='#'){
            ret->us.v.hidden = 1;
        }
        if(parent->us.v.system || parent->ut.v.suffixType != SF_DIR){
            ret->us.v.system = 1;
        }else if(len==7 && memcmp("Library",filename,7)==0){
            ret->us.v.system = 1; 
        } if(parent->us.v.FileNameLength==0){
            if(strcmp(filename,"Users")!=0 && strcmp(filename,"Volumes")!=0) ret->us.v.system = 1; 
        }
    }
	addChildren(parent,ret);
	set_time(ret, statptr->st_mtime);
	if(IsDir(ret)){
		SET_SIZE(ret,0);
	}else{
		SET_SIZE(ret,file_size_shorten(statptr->st_size));
	}
	ALL_FILE_COUNT +=1;
	return ret;
}

BOOL ignore_dir(char *fullpath, char *filename){
    int filenamelen = strlen(filename);
    if(filenamelen==3 && strncmp(filename,"CVS",3)==0) return 1;
    if(filenamelen==4 && strncmp(filename,".git",4)==0) return 1;
    if(filenamelen==4 && strncmp(filename,".svn",4)==0) return 1;
#ifdef APPLE
    if(strncmp(fullpath,"/private",8)==0) return 1;
#else
    if(strncmp(fullpath,"/tmp",4)==0) return 1;
#endif
    return 0;
}

BOOL ignore_dir2(char *dir_name){
    char *dir_only_name = strrchr(dir_name,'/')+1;
    if(*dir_only_name=='\0'){
        *(dir_only_name-1) = '\0';
        dir_only_name = strrchr(dir_name,'/')+1;
    }
    return ignore_dir(dir_name,dir_only_name);
}


static char *fullpath;

static void dopath(char *filename, pFileEntry parent);

static void dirent_visitor(char *dir_name, struct dirent * dp, va_list ap){
    char *last_slash = va_arg(ap, char *);
    pFileEntry parent = va_arg(ap, pFileEntry);
    //printf("%s, %x, %x, %s\n",fullpath,last_slash, fullpath, dp->d_name);
    strcpy(last_slash, dp->d_name);	/* append name after slash */
    dopath(dp->d_name,parent);    
}

static void dopath(char *filename, pFileEntry parent){
	struct stat		statbuf;
	if (lstat(fullpath, &statbuf) >= 0){
		pFileEntry self;
        if(*filename!='\0'){
            self = initUnixFile(&statbuf, filename,parent);
        }else{
            self = parent;
        }
        if(ignore_dir(fullpath,filename)) return;
		if(S_ISDIR(statbuf.st_mode)) {
            char *ptr;
			ptr = fullpath + strlen(fullpath);	/* point to end of fullpath */
			*ptr++ = '/';
			*ptr = '\0';
            dir_iterate(dirent_visitor, fullpath, ptr, self);
			ptr[-1] = '\0';	/* erase everything from slash onwards */
		}
	}
}


int scanUnix(pFileEntry root, int i){
	long len = pathconf("/", _PC_PATH_MAX);
	fullpath = (char *)malloc_safe(len);
	strncpy(fullpath, "/", len);
	fullpath[len-1] = 0;
	printf("%d ,%s\n",len,fullpath);
	dopath("",root);
	free_safe(fullpath);
	return ALL_FILE_COUNT;
}
