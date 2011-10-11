#include "server.h"
#include "error.h"
#include "sharelib.h"
#include "server.h"
#include "util.h"
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


volatile static BOOL ShutDown = 0;

static int					listenfd, connfd;
static pid_t				childpid;
static socklen_t			clilen;
static struct sockaddr_un	cliaddr, servaddr;

void sig_chld(int signo){
	pid_t	pid;
	int		stat;

	while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0) {
		printf("child %d terminated\n", pid);
	}
	return;
}

BOOL start_named_pipe(){
	listenfd = socket(AF_LOCAL, SOCK_STREAM, 0);
	if(listenfd<0) err_quit("unix domain socket error");
	unlink(UNIXSTR_PATH);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_LOCAL;
	strcpy(servaddr.sun_path, UNIXSTR_PATH);

	if( bind(listenfd, (SA *) &servaddr, sizeof(servaddr)) <0 ) err_quit("bind error");
	if( listen(listenfd, LISTENQ) <0) err_quit("listen error");
	signal(SIGCHLD, sig_chld);
	return 1;

}

void wait_stop_named_pipe(){
	while(!ShutDown){
		clilen = sizeof(cliaddr);
		if ( (connfd = accept(listenfd, (SA *) &cliaddr, &clilen)) < 0) {
			if (errno == EINTR)
				continue;		/* back to for() */
			else
				err_sys("accept error");
		}
		if ( (childpid = fork()) == 0) {	/* child process */
			SearchRequest req;
			close(listenfd);	/* close listening socket */
			while (read (connfd, &req, sizeof(SearchRequest) )>0 ) { 
				process(req,connfd);
			}
			exit(0);
		}
		close(connfd);			/* parent closes connected socket */
	}	
}

BOOL shutdown_handle(DWORD CtrlEvent){
	ShutDown = 1;
	return 1;
}