#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3490"
#define BACKLOG 10

//to get socket internet address
void *getInAddr(struct sockaddr *sa){
	if(sa->sa_family==AF_INET){
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[]){
	int sockfd;
	struct addrinfo server_addr;
	struct addrinfo  *res;

	memset(&server_addr,0, sizeof(server_addr) );
	server_addr.ai_family= AF_UNSPEC;
	server_addr.ai_socktype= SOCK_STREAM;
	server_addr.ai_flags= AI_PASSIVE;
	int error_check;
	if ((error_check=getaddrinfo(NULL,PORT,&server_addr,&res))!=0){
		fprintf(stderr, "getaddrinfo: %s\n",gai_strerror(error_check));
		return 1;
	}
	struct addrinfo *i;
	for(i=res; i!=NULL; i=i->ai_next){
		if((sockfd=socket(i->ai_family,i->ai_socktype,i->ai_protocol))==-1){
			perror("server:socket");
			continue;
		}
		//can use setsockopt to reuse a already binded port
		if (bind(sockfd, i->ai_addr, i->ai_addrlen) == -1){
			close(sockfd);
			perror("server:bind");
			continue;
		}
		break;
	}
	freeaddrinfo(res);

	if (i ==NULL){
		fprintf(stderr,"server: failed to biind \n");
		exit(1);
	}
	if(listen(sockfd,BACKLOG)==-1){
		perror("listen");
		exit(1);
	}
	printf("server: waiting for connections....\n");

	struct sockaddr_storage client_addr;
	socklen_t sin_size;
	char client_ip[INET6_ADDRSTRLEN];
	char *msg="Hello from Server";
	while(1){
		sin_size=sizeof(client_addr);
		int new_fd= accept(sockfd,(struct sockaddr *)&client_addr,&sin_size);
		if(new_fd==-1){
			perror("Accept");
			continue;
		}

		inet_ntop(client_addr.ss_family,getInAddr((struct sockaddr *)&client_addr),client_ip,sizeof(client_ip));
		printf("Server received connection from %s\n",client_ip);

		if(!fork()){
			close(sockfd);
			if(send(new_fd,msg,strlen(msg),0)==-1)
				perror("send");
			close(new_fd);
			exit(0);
		}
		close(new_fd);
	}
	return 0;
}
