#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define PORT "3490"
#define MAXDATA 100

//to get the socket internet address
void *getInAddr(struct sockaddr *sa){
	if(sa->sa_family==AF_INET){
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main (int argc, char *argv[]){
	int sockfd;
	struct addrinfo client_addr, *res, *i;

	memset(&client_addr,0,sizeof(client_addr));
	client_addr.ai_family=AF_UNSPEC;
	client_addr.ai_socktype=SOCK_STREAM;

	int error_check;
	if((error_check=getaddrinfo(argv[1],PORT,&client_addr,&res))!=0){
		fprintf(stderr, "getaddrinfo: %s\n",gai_strerror(error_check));
		return 1;
	}
	for (i=res; i!=NULL; i=i->ai_next){
		if((sockfd=socket(i->ai_family,i->ai_socktype,i->ai_protocol))==-1){
			perror("Client: socket");
			continue;
		}
		if((error_check=connect(sockfd, i->ai_addr, i->ai_addrlen))==-1){
			close(sockfd);
			perror("client: connect");
			continue;
		}
		break;
	}
	if(i==NULL){
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}
	char server_ip[INET6_ADDRSTRLEN];
	inet_ntop(i->ai_family, getInAddr((struct sockaddr *)i->ai_addr),server_ip,sizeof server_ip);
	printf ("Client connecting to %s\n", server_ip);

	freeaddrinfo(res);
	int numbytes;
	char buf[MAXDATA];
	if((numbytes=recv(sockfd,buf,MAXDATA-1,0))==-1){
		perror("recv");
		exit(1);
	}

	buf[numbytes]='\0';

	printf("Client : received '%s'\n",buf);

	close(sockfd);
	return 0;
}
