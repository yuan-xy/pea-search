#include "server.h"

volatile static BOOL ShutDown = FALSE;

BOOL start_named_pipe(){
	int					listenfd, connfd;
	pid_t				childpid;
	socklen_t			clilen;
	struct sockaddr_un	cliaddr, servaddr;
	void				sig_chld(int);
	
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
			close(listenfd);	/* close listening socket */
			str_echo(connfd);	/* process the request */
			exit(0);
		}
		close(connfd);			/* parent closes connected socket */
	}	
}

BOOL shutdown_handle(DWORD CtrlEvent){
	ShutDown = 1;
	return 1;
}