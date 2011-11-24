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
    if (dp->d_namlen == file->us.v.FileNameLength && !strcmp(dp->d_name, file->FileName)) {
        return 1;
    }   
    return 0;
}

void dir_iterate(char *dir_name, pDirentVisitor visitor, void *data){
    struct dirent * dp;
    DIR * dirp = opendir(dir_name);
    if(dirp==NULL) return;
    /*
     TODO: The scandir(3) function returns an array of directory entries that you can quickly iterate through. 
     This is somewhat easier than reading a directory manually with opendir(3), readdir(3), and so on, and is slightly more efficient since you will always iterate through the entire directory while caching anyway.
     */
    while ((dp = readdir(dirp)) != NULL){
        if (strcmp(dp->d_name, ".") == 0  || strcmp(dp->d_name, "..") == 0) continue;
        (*visitor)(dir_name,dirp,dp,data);
    }
    closedir(dirp);
}

BOOL dir_iterateB(char *dir_name, pDirentVisitorB visitor, void *data, char *buffer){
    struct dirent * dp;
    DIR * dirp = opendir(dir_name);
    if(dirp==NULL) return NULL;
    while ((dp = readdir(dirp)) != NULL){
        if (strcmp(dp->d_name, ".") == 0  || strcmp(dp->d_name, "..") == 0) continue;
        BOOL flag = (*visitor)(dir_name,dirp,dp,data);
        if(flag){
            if(buffer!=NULL) strcpy(buffer, dp->d_name);
            closedir(dirp); 
            return 1; 
        }
    }
    closedir(dirp); 
    return 0;
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
struct scan_context{
    char *last_slash;
    pFileEntry parent;
};
typedef struct scan_context ScanContext, *pScanContext;


static void dopath(char *filename, pFileEntry parent);

static void dirent_visitor(char *dir_name, DIR * dirp, struct dirent * dp, void *data){
    pScanContext ctx = (pScanContext)data;
    strcpy(ctx->last_slash, dp->d_name);	/* append name after slash */
    dopath(dp->d_name,ctx->parent);    
}

static void dopath(char *filename, pFileEntry parent){
	struct stat		statbuf;
    //printf("%s, %s, %s\n",fullpath,last_slash, filename);
	if (lstat(fullpath, &statbuf) >= 0){
		pFileEntry self;
        if(*filename!='\0'){
            self = initUnixFile(&statbuf, filename,parent);
        }else{
            self = parent;
        }
        if(ignore_dir(fullpath,filename)) return;
		if(S_ISDIR(statbuf.st_mode)) {
            ScanContext ctx;
            char *ptr;
			ptr = fullpath + strlen(fullpath);	/* point to end of fullpath */
			*ptr++ = '/';
			*ptr = '\0';
            ctx.last_slash = ptr;
            ctx.parent = self;
            dir_iterate(fullpath, dirent_visitor, &ctx);
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
