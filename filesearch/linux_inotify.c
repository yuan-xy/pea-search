#include "env.h"
#include "global.h"
#include "fs_common.h"
#include "posixfs.h"
#include <dirent.h>
#include <limits.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )


static pthread_t ntid;
static int notifyfd;
static int wd;

static void init(void *arg){
	notifyfd = inotify_init();
	if(notifyfd < 0) {
        perror("inotify_init failed");
	    return;
	}
    wd = inotify_add_watch( notifyfd, "/Users/ylt/", IN_MODIFY | IN_CREATE | IN_DELETE );
    if(wd==-1) perror( "inotify_add_watch error" );
    
    while(1){
        int i=0;
        char buffer[BUF_LEN];
        int length = read( notifyfd, buffer, BUF_LEN );  
        if ( length < 0 ) {
            perror( "read" );
            return;
        } 
        while ( i < length ) {
            struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
            if ( event->len ) {
                if ( event->mask & IN_CREATE ) {
                    if ( event->mask & IN_ISDIR ) {
                        printf( "The directory %s was created.\n", event->name );       
                    }
                    else {
                        printf( "The file %s was created.\n", event->name );
                    }
                }
                else if ( event->mask & IN_DELETE ) {
                    if ( event->mask & IN_ISDIR ) {
                        printf( "The directory %s was deleted.\n", event->name );       
                    }
                    else {
                        printf( "The file %s was deleted.\n", event->name );
                    }
                }
                else if ( event->mask & IN_MODIFY ) {
                    if ( event->mask & IN_ISDIR ) {
                        printf( "The directory %s was modified.\n", event->name );
                    }
                    else {
                        printf( "The file %s was modified.\n", event->name );
                    }
                }
            }
            i += EVENT_SIZE + event->len;
        }
    }
}

BOOL StopMonitorThreadLinux(int i){
	close(notifyfd);
    return pthread_kill(ntid,9);
}

BOOL StartMonitorThreadLinux(int i){
	return pthread_create(&ntid,NULL,init,&i);
}
