#include "env.h"
#include "sharelib.h"
#include "common.h"
#include <stdio.h>
#include <locale.h>
#include	<sys/types.h>	/* basic system data types */
#include	<sys/socket.h>	/* basic socket definitions */
#include	<sys/time.h>	/* timeval{} for select() */
#include	<time.h>		/* timespec{} for pselect() */
#include	<netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include	<arpa/inet.h>	/* inet(3) functions */
#include	<errno.h>
#include	<fcntl.h>		/* for nonblocking */
#include	<netdb.h>
#include	<signal.h>
#include	<sys/stat.h>	/* for S_xxx file mode constants */
#include	<sys/uio.h>		/* for iovec{} and readv/writev */
#include	<unistd.h>
#include	<sys/wait.h>
#include	<sys/un.h>		/* for Unix domain sockets */
#include "unix_domain_client.h"


BOOL connect_unix_socket(int *psock) {
	int	sockfd;
	sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);
	if(sockfd<0) {
		printf("unix domain socket error");
		return 0;
	}else{
		struct sockaddr_un	servaddr;
		bzero(&servaddr, sizeof(servaddr));
		servaddr.sun_family = AF_LOCAL;
		strcpy(servaddr.sun_path, UNIXSTR_PATH);
		if(connect(sockfd, (SA *) &servaddr, sizeof(servaddr))!=0 ) {
			printf("connect error");
			return 0;
		}else{
			*psock = sockfd;
			return 1;
		}
	}
}

static ssize_t read_all(int fildes, char *buf, size_t nbyte){
    int read_bytes=0;
    char *buffer = buf;
    do{
        int ret = read(fildes,buffer,nbyte);
        if(ret<=0) return ret;
        read_bytes+=ret;
        buffer+=ret;
    }while(read_bytes<nbyte);
    return read_bytes;
}

BOOL query(int sockfd, SearchRequest *req, char *buffer){
	SearchResponse resp;
    if (write(sockfd, req, sizeof(SearchRequest))<=0) {
        printf("scoket write error");
		strcpy(buffer,"error");
		return 0;
    }
    if(read(sockfd, &resp, sizeof(int))>0){
        DWORD len = resp.len;
        ssize_t err;
        err=read_all(sockfd, buffer, len);
        printf("---len:%d, read:%d\n", len,err);
        if(err<=0){
            printf("scoket read error.\n");
            strcpy(buffer,"error");
			return 0;
        }
		return 1;
    }else{
        strcpy(buffer,"error");
		return 0;
    }
}
