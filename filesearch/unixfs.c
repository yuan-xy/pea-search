#include "env.h"
#include "global.h"
#include "fs_common.h"
#include "util.h"
#include "suffix.h"
#include "search.h"
#include <dirent.h>
#include <limits.h>

static pFileEntry initUnixFile(const struct stat *statptr, char *filename, pFileEntry parent, int i){
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

BOOL ignore_file(char *fullpath, char *filename){
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

static void dopath(char *fullpath, char *filename, pFileEntry parent, int i){
	struct stat		statbuf;
	if (lstat(fullpath, &statbuf) >= 0){
		pFileEntry self;
        if(*filename!='\0'){
            self = initUnixFile(&statbuf, filename,parent,i);
        }else{
            self = parent;
        }
        if(ignore_file(fullpath,filename)) return;
		if(S_ISDIR(statbuf.st_mode)) {
			char *ptr;
			DIR *dp;
			ptr = fullpath + strlen(fullpath);	/* point to end of fullpath */
			*ptr++ = '/';
			*ptr = 0;
			if ((dp = opendir(fullpath)) != NULL) {
/*
 TODO: The scandir(3) function returns an array of directory entries that you can quickly iterate through. 
       This is somewhat easier than reading a directory manually with opendir(3), readdir(3), and so on, and is slightly more efficient since you will always iterate through the entire directory while caching anyway.
*/
				struct dirent	*dirp;
				while ((dirp = readdir(dp)) != NULL) {
					if (strcmp(dirp->d_name, ".") == 0  || strcmp(dirp->d_name, "..") == 0) continue;
					strcpy(ptr, dirp->d_name);	/* append name after slash */
					dopath(fullpath, dirp->d_name,self,i);
				}
				ptr[-1] = 0;	/* erase everything from slash onwards */
				if (closedir(dp) < 0) printf("can't close directory %s", fullpath);
			}
		}
	}
}


int scanUnix(pFileEntry root, int i){
	long len = pathconf("/", _PC_PATH_MAX);
	char *fullpath = (char *)malloc_safe(len);
	strncpy(fullpath, "/", len);
	fullpath[len-1] = 0;
	printf("%d ,%s\n",len,fullpath);
	dopath(fullpath,"",root,i);
	free_safe(fullpath);
	return ALL_FILE_COUNT;
}
