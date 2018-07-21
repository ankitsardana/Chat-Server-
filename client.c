#include <stdio.h>		//standard C i/o facilities
#include <string.h>		//for handling the string related function.
#include <stdlib.h>		//for handling the standard library functions
#include <sys/types.h>		//header file include definitions for different types:
#include <sys/socket.h>		//for defining the generic socket address structure
#include <netinet/in.h>		//for defining the socket address structure
#include <arpa/inet.h>		//IP address conversion stuff	
#include <unistd.h>		//defines STDIN_FILENO, system calls,etc
#include <errno.h>		//it defines macros for reporting and retrieving error conditions


#define BUFSIZE 2048


void send_recv(int i, int sockfd)
{
	/*
	Objective : Send and receive a message 
	Input parameters : i - file descriptor
					   sockfd : socket file descriptor
	*/
	char send_buf[BUFSIZE];
	char recv_buf[BUFSIZE];
	int nbyte_recvd;
	
	if (i == 0)
	{	
		fgets(send_buf, BUFSIZE, stdin);
		if (strcmp(send_buf , "quit\n") == 0) {
			exit(0);
		}
		else
			send(sockfd, send_buf, strlen(send_buf), 0);
	}else
	{
		printf("-->");
		nbyte_recvd = recv(sockfd, recv_buf, BUFSIZE, 0);
		recv_buf[nbyte_recvd] = '\0';
		printf("%s\n" , recv_buf);
		fflush(stdout);
	}

}

		
void connect_request(int *sockfd, struct sockaddr_in *server_addr)
{
	/*
	Objective : Establish connection with server
	Input parameters : *sockfd : pointer to socket descriptor
					   *server_addr : pointer to server address	   
	*/
	if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Socket");
		exit(1);

	}
	server_addr->sin_family = AF_INET;
	server_addr->sin_port = htons(8888);
	server_addr->sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(server_addr->sin_zero, '\0', sizeof server_addr->sin_zero);
	if(connect(*sockfd, (struct sockaddr *)server_addr, sizeof(struct sockaddr)) == -1) 
	{
		perror("connect");
		exit(1);

	}
}

int main()
{
	int sockfd, fdmax, i;
	struct sockaddr_in server_addr;
	fd_set master;
	fd_set read_fds;

	connect_request(&sockfd, &server_addr);
	FD_ZERO(&master);
        FD_ZERO(&read_fds);
        FD_SET(0, &master);
        FD_SET(sockfd, &master);

	fdmax = sockfd;

	while(1)
	{
		read_fds = master;
		if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)
		{
			perror("select");
			exit(4);
		}

		for(i=0; i <= fdmax; i++ )
			if(FD_ISSET(i, &read_fds))
				send_recv(i, sockfd);
	}

	printf("client-quited\n");
	close(sockfd);
	return 0;
}
