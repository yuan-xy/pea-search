#include "env.h"
#include "global.h"
#include "fs_common.h"
#include "util.h"
#include "suffix.h"
#include "search.h"
#include "unixfs.h"
#include <dirent.h>
#include <limits.h>

#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>

static FSEventStreamRef _stream;
static FSEventStreamContext *_context;
static BOOL _running=0;
static pthread_t ntid;

void add_file_visitor(char *dir_name, DIR * dirp, struct dirent * dp, void *data){
    pFileEntry dir = (pFileEntry)data;
    if(SubDirIterateB(dir, (pFileVisitorB)same_file, dp)==NULL){
        struct stat		statbuf;
        char buffer[MAX_PATH],*p=buffer;
        p=stpcpy(p,dir_name);
        p=stpcpy(p,"/");
        p=stpcpy(p,dp->d_name);
        if (lstat(buffer, &statbuf) >= 0){
            pFileEntry pf = initUnixFile(&statbuf,dp->d_name,dir);
            printf("add file to tree: %s\n", pf->FileName);
        }
    }
}

static void add_file(char * dir_name){
    if(ignore_dir2(dir_name)){
        printf("ignore dir: %s\n", dir_name);
        return; 
    }else{
        pFileEntry dir = find_file(dir_name,strlen(dir_name));
        if(dir==NULL) return;
        dir_iterate(dir_name, add_file_visitor, dir);
    }
}

static BOOL same_file_visitor(char *dir_name, DIR * dirp, struct dirent * dp, void *data){
    pFileEntry file = (pFileEntry)data;
    return same_file(file,dp);
}

static void remove_file_visitor(pFileEntry file, void *data){
    char * dir_name = (char *)data;
    if(!dir_iterateB(dir_name, same_file_visitor, file, NULL)){
        printf("delete file: %s\n", file->FileName);
        deleteFile(file);
    }
}

static void remove_file(char * dir_name){
    if(ignore_dir2(dir_name)){
        printf("ignore dir: %s\n", dir_name);
        return; 
    }else{
        pFileEntry dir = find_file(dir_name,strlen(dir_name));
        if(dir==NULL) return;
        SubDirIterate(dir, remove_file_visitor, dir_name);
    }
}

static void feCallback(ConstFSEventStreamRef streamRef, void *clientCallBackInfo, size_t numEvents, void *eventPaths, const FSEventStreamEventFlags eventFlags[], const FSEventStreamEventId eventIds[]) {	
    char **paths = eventPaths;
    int i;
    printf("----%lu----\n",numEvents);
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
            remove_file(paths[i]);
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
