#include "env.h"
#include "global.h"
#include "fs_common.h"
#include "util.h"
#include "suffix.h"
#include "search.h"
#include <dirent.h>
#include <limits.h>

#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>

static FSEventStreamRef _stream;
static FSEventStreamContext *_context;
static BOOL _running=0;
static pthread_t ntid;

BOOL same_file(pFileEntry file, struct dirent * dp){
    if (dp->d_namlen == file->us.v.FileNameLength && !strcmp(dp->d_name, file->FileName)) {
        return 1;
    }   
    return 0;
}

static BOOL find_file_in_cur_dir(pFileEntry dir, struct dirent * file){
    return SubDirIterateB(dir, (pFileVisitorB)same_file, file)!=NULL;
}

static void add_file(char * dir_name){
    DIR * dirp = opendir(dir_name);
    struct dirent * dp;
    pFileEntry dir = find_file(dir_name,strlen(dir_name));
    if(dir==NULL) return;
    while ((dp = readdir(dirp)) != NULL){
        if (strcmp(dp->d_name, ".") == 0  || strcmp(dp->d_name, "..") == 0) continue;
        if(!find_file_in_cur_dir(dir,dp)){
            //add_file(dp->d_name,dp->d_namlen,0);
            printf("***add file: %s\n",dp->d_name);
        }
    }
    closedir(dirp);
}
static void feCallback(ConstFSEventStreamRef streamRef, void *clientCallBackInfo, size_t numEvents, void *eventPaths, const FSEventStreamEventFlags eventFlags[], const FSEventStreamEventId eventIds[]) {	
    char **paths = eventPaths;
    int i;
    printf("----%d----\n",numEvents);
    for (i = 0; i < numEvents; i++) {
        FSEventStreamEventFlags flag = eventFlags[i];
        if (flag & kFSEventStreamEventFlagRootChanged) return;
        if (flag & kFSEventStreamEventFlagMustScanSubDirs) return;
        if (flag & kFSEventStreamEventFlagItemCreated && 
            flag & kFSEventStreamEventFlagItemRenamed){
            printf("create rename ");  
            printf("%04x: %s\n", flag, paths[i]);
            add_file(paths[i]);
        }else if (flag & kFSEventStreamEventFlagItemCreated){
            printf("create ");  
            printf("%04x: %s\n", flag, paths[i]);
            add_file(paths[i]);
        }else if (flag & kFSEventStreamEventFlagItemRemoved){
            printf("remove ");
            printf("%04x: %s\n", flag, paths[i]);
        }else if (flag & kFSEventStreamEventFlagItemInodeMetaMod){
            printf("meta ");
            printf("%04x: %s\n", flag, paths[i]);
        }else if (flag & kFSEventStreamEventFlagItemRenamed){
            if(i+1<numEvents && eventFlags[i+1]==flag){
                printf("move ");
                printf("%04x: from: %s\n", flag, paths[i]);
                printf(" to %s\n",paths[i+1]);
                i+=1;
            }else{
                printf("rename ");
                printf("%04x: %s\n", flag, paths[i]);
            }
        }else if (flag & kFSEventStreamEventFlagMount){
            printf("mount ");
            printf("%04x: %s\n", flag, paths[i]);
        }else if (flag & kFSEventStreamEventFlagUnmount){
            printf("unmount ");
            printf("%04x: %s\n", flag, paths[i]);
        }
   }
}

static void init(int i){
	printf("start fsevent on / \n");
	CFStringRef path = CFSTR("/");
	CFArrayRef pathsToWatch = CFArrayCreate(NULL, (const void **)&path, 1, NULL);
	/* Use context only to simply pass the array controller */
	_context = (FSEventStreamContext*)malloc(sizeof(FSEventStreamContext));
	_context->version = 0;
	//_context->info = (void*)ctrl; 
	_context->retain = NULL;
	_context->release = NULL;
	_context->copyDescription = NULL;
    
	_stream = FSEventStreamCreate(NULL,
	    &feCallback,
	    _context,
	    pathsToWatch,
	    kFSEventStreamEventIdSinceNow, /* Or a previous event ID */
	    1.0, /* Latency in seconds */
	    kFSEventStreamCreateFlagNone
	);
	FSEventStreamScheduleWithRunLoop(_stream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);    
	_running = FSEventStreamStart(_stream);
	if(_running) CFRunLoopRun();
	printf("run fsevent on / with status:%d\n",_running);
	// Although it's not strictly necessary, make sure we see any pending events... 
    FSEventStreamFlushSync(_stream);
    FSEventStreamStop(_stream);
	printf("exit fsevent on / with status:%d\n",_running);
}

BOOL StopMonitorThreadMAC(int i){
	    if(_running) FSEventStreamStop(_stream);
	     FSEventStreamInvalidate(_stream); /* will remove from runloop */
	     FSEventStreamRelease(_stream);
	     free(_context);
		pthread_kill(ntid,9);
}

BOOL StartMonitorThreadMAC(int i){
	pthread_create(&ntid,NULL,init,&i);
}

static pFileEntry initMacFile(const struct stat *statptr, char *filename, pFileEntry parent, int i){
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


static void dopath(char *fullpath, char *filename, pFileEntry parent, int i){
	struct stat		statbuf;
	if (lstat(fullpath, &statbuf) >= 0){
		pFileEntry self;
        if(*filename!='\0'){
            self = initMacFile(&statbuf, filename,parent,i);
        }else{
            self = parent;
        }
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


int scanMac(pFileEntry root, int i){
	long len = pathconf("/", _PC_PATH_MAX);
	char *fullpath = (char *)malloc_safe(len);
	strncpy(fullpath, "/", len);
	fullpath[len-1] = 0;
	printf("%d ,%s\n",len,fullpath);
	dopath(fullpath,"",root,i);
	free_safe(fullpath);
	return ALL_FILE_COUNT;
}
