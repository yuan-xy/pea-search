#include "env.h"
#include "../filesearch/sharelib.h"
#include <stdio.h>
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


int main(int argc, char * argv[]) {
	int	sockfd;
	SearchRequest req;
	SearchResponse resp;
	if(!connect_unix_socket(&sockfd)) return 1;
	memset(&req,0,sizeof(SearchRequest));
	req.from = 0;
	req.rows = 3;
	do{
		fgetws(req.str,63,stdin);
		if(req.str[0]==L'q') break;
		if (write(sockfd, &req, sizeof(SearchRequest))<=0) {
			printf("scoket write error");
			return 1;
		}
		if(read(sockfd, &resp, sizeof(int))>0){
			char buffer[1024];
			DWORD len = resp.len;
			printf("%d,", len);
			memset(buffer,(char)0,1024);
			if(read(sockfd, buffer, len)<=0){
				printf("scoket read error.");
			}
			printf("%s\n", buffer);
		}else{
			printf("scoket read error");
			return 1;
		}
	}while(1);
    printf("existing...");
    fflush(stdout);
	return 0;
}

