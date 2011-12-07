#include "env.h"
#include "../filesearch/sharelib.h"
#include <stdio.h>
#include <locale.h>
#include "unix_domain_client.h"

int main(int argc, char * argv[]) 
{
	int	sockfd;
	SearchRequest req;
	char buffer[1024];
    setlocale(LC_ALL, "");
	if(!connect_unix_socket(&sockfd)) return 1;
	memset(&req,0,sizeof(SearchRequest));
	req.from = 0;
	req.rows = 3;
	do{
		fgetws(req.str,63,stdin);
		if(req.str[0]==L'q') break;
		query(sockfd, &req, buffer);
		printf("%s\n", buffer);
	}while(1);
    printf("existing...");
    fflush(stdout);
	return 0;
}

