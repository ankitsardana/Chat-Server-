/*					            Unix Network Programming Assignment
                                                       
						       Submitted to:- 
                                                                       Nisha
                                                       
						       Submitted by:-
                                                                        Ankit Sardana(04)
									                                                                     
									Rishabh Joshi(27)
									
									Prabal Partap(18)
                                                                 


Problem Statement  – 
			Chat server is a stand-alone application that is made by the combination of two-application, server application
 (which runs on server side) and client application (which runs on client side). This application is used  for chatting in stand-alone
 system using loop back network.
	In general chat server is an application that does the following:
	Listens requests for incoming calls from clients.
	Listens messages from all the connected clients.
	Send messages from clients to other clients connected to the server.
	Chat client does the following:
	Send messages to server as well as to other connected clients in the network.

Objective:
The aim of this project is to express how we can implement a simple chat application between a server and a client. The application is a
 console based application and is implemented by using Unix Sockets. The project is developed in C/C++ language and can be executed on a
 single stand-alone UNIX machine across a network using loop back address concept.
	Application consists of two programs:
	Server
	Client
	

TO RUN: 
$ gcc progf.c -o progf
$ ./progf   		*/






#include<netinet/in.h>			//for defining the socket address structure
#include<sys/socket.h>			//for defining the generic socket address structure
#include<arpa/inet.h>			//IP address conversion stuff		
#include<unistd.h>			//defines STDIN_FILENO, system calls,etc
#include<string.h>			//for handling the string related function.
#include<stdio.h>			//standard C i/o facilities
#include<stdlib.h>			//for handling the standard library functions
#include<errno.h>			//it defines macros for reporting and retrieving error conditions

#define SERV_PORT 8888
#define LOCALHOST 2130706433
struct conn_entry{
	char username[20];
	int sockfd;
	
};								//Online connections-----Persons' online

struct user_entry{
	char username[20];
	char password[8];
};								//Registered entries-----Persons' allowed to chat



void create_conn_entry(FILE *fp , struct conn_entry entry){
	/*
	Objective : Store a connection entry in a file
	Input parameters : fp - file pointer of file containing online connection entries
					   entry - filled connected entry  
	*/
	fseek(fp , 0 ,SEEK_END);
	fwrite(&entry, sizeof(struct conn_entry) , 1 , fp);
}

void remove_conn_entry(FILE *fp , int sock_fd){
	/*
	Objective : Remove a connection entry from a file
	Input parameters : fp - file pointer of file containing online connection entries
					   sock_fd - socket file descriptor of connection  
	*/
	struct conn_entry connection;
	rewind(fp);
	while(fread(&connection , sizeof(struct conn_entry) , 1 , fp) == 1){
		if(sock_fd == connection.sockfd){
			connection.sockfd =0;
			
			fseek(fp , -(sizeof(struct conn_entry)) , SEEK_CUR);
			fwrite(&connection , sizeof(struct conn_entry) ,1 ,fp);
			break;
		}	
	}
}

int check_username(FILE *fp , char username[] , int fd){
	/*
	Objective : Check if a username is online 
	Input parameters : fp - file pointer of file containing online connection entries
					   fd - socket file descriptor of sender 
					   username - username of receiver
	Return type : -1 : sender and receiver are same
				   socket fd of receiver : Found 	   
	*/
	rewind(fp);
	struct conn_entry connection;
	while(fread(&connection , sizeof(struct conn_entry) , 1 , fp) == 1){
		if(strcmp(username , connection.username) == 0){
			if(connection.sockfd == fd)
				return -1;
			else
				return (int)connection.sockfd;
		}
	}
	return 0;
}

int check_online_entry(FILE *fp , struct user_entry entry){
	/*
	Objective : Check if a registered entry is online 
	Input parameters : fp - file pointer of file containing online connection entries
					   entry : Filled in registered entry
	Return type : 0 : Not online
				  1 : Found online	   
	*/
	struct conn_entry connection;
	rewind(fp);
	while(fread(&connection , sizeof(struct conn_entry) , 1 , fp) == 1){
		if(strcmp(connection.username,entry.username) == 0)
			return 1;
	}
	return 0;
}




int check_data_entry(FILE *fp , struct user_entry entry){
	/*
	Objective : Check if a registered entry is online 
	Input parameters : fp - file pointer of file containing online connection entries
					   entry : Filled in registered entry
	Return type : 0 : Not online
				  1 : Found online	   
	*/
	rewind(fp);
	struct user_entry connection;
	while(fread(&connection , sizeof(struct user_entry) , 1 , fp) == 1){
		if((strcmp(entry.username , connection.username) == 0)&&(strcmp(entry.password,connection.password)==0)){
			return 1;
		}
	}
	return 0;
}





void list_online_connections(FILE *fp){
	/*
	Objective : list connections that are online 
	Input parameters : fp - file pointer of file containing online connection entries
					   	   
	*/
	rewind(fp);
	struct conn_entry connection;

	while(fread(&connection , sizeof(struct conn_entry) , 1 , fp) == 1)
		if(connection.sockfd != 0)
			printf("\n%s \n",connection.username);	
}

void list_connections(FILE *fp){
	/*
	Objective : list connections that are registered 
	Input parameters : fp - file pointer of file containing registered connection entries
					   	   
	*/
	rewind(fp);
	struct user_entry connection;

	while(fread(&connection , sizeof(struct user_entry) , 1 , fp) == 1)
		printf("\n%s\n",connection.username);	
}

int main(int argc, char **argv)
{
	int listenfd, connfd, client_socket[30], max_clients = 30, opt = 1,  activity , max_sd,sd;
	socklen_t  clilen;
	struct sockaddr_in  cliaddr, servaddr;
	fd_set readfds;
	char s_buf[1024];
	char r_buf[1024];
	struct user_entry entry;
	struct conn_entry connection;
	
	FILE *ue_p, *ce_p;

	if((ue_p = fopen("User_data.dat","r")) == NULL ){
		printf("Cannot open file");
	} 


	if((ce_p = fopen("Conn_data.dat","wb+")) == NULL ){
		printf("Cannot open file");
	} 

	//Creating a socket
	
	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("socket failed..Socket error");
	}

	for (int i = 0; i < max_clients; ++i)
	{
		client_socket[i] = 0;
	}

	//To set options at the socket level and set option to allow use of local addresses

	if(setsockopt(listenfd , SOL_SOCKET , SO_REUSEADDR , (char *)&opt , sizeof(opt)) < 0){
		perror("setsocketopt :");
		exit(0);
	}

	//To set options at the socket level

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port        = htons(SERV_PORT);

	//Binding a socket
	bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
	
	
	//Listening at 8888
	printf("<-->Listener port : %d\n",SERV_PORT);
	if(listen(listenfd, 3) < 0){
		perror("Listen");
		exit(0);
	}

	
	puts("<-->Waiting for connections\n");
	printf("<-->Registered connections\n");
	list_connections(ue_p);

	while(1){
		memset(s_buf, 0 ,sizeof(s_buf));
		memset(r_buf, 0 ,sizeof(r_buf));
		

		FD_ZERO(&readfds);
		FD_SET(listenfd , &readfds);
		max_sd = listenfd;
		
		//Check if there is a new connection in client socket array

		for (int i = 0; i < max_clients; ++i)
		{
			sd = client_socket[i];
			if(sd > 0)
				FD_SET(sd , &readfds);				//add to read file descriptors		
			
			if(sd > max_sd)
				max_sd = sd;
		}

				
		activity = select(max_sd+1 , &readfds , NULL , NULL , NULL); 		//Check for activity
		if(activity < 0 && errno != EINTR){
			printf("Select error");

		}
		
		if(FD_ISSET(listenfd , &readfds))					// if server is set
		{
			memset(s_buf, 0 ,sizeof(s_buf));
			memset(r_buf, 0 ,sizeof(r_buf));
			
			printf("<-->Incoming connection!!!!!!\n");
			clilen = sizeof(cliaddr);

			//Accept the connection
			if((connfd = accept(listenfd , (struct sockaddr*)&cliaddr , &clilen)) < 0){
				perror("accept");
				exit(0);
			}
			

			printf("<-->New connection , socket fd is %d IP adress %s : port %d\n",connfd,inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port));    
			int is_entry_valid , is_entry_online; 
			int flag1=0;
			do
			{			
			int n;
			if(flag1 == 0)
				strcpy(s_buf,"Please enter your username and password\n");
			else if(flag1 == 1)
				strcpy(s_buf,"---------------Sorry username and password does not match. Please enter correct details--------------\n");
			else
				strcpy(s_buf,"---------------Username is online...Login with another id-------------\n");
			if(send(connfd , s_buf , sizeof(s_buf), 0) < 0)
				perror("Send error\n");
			memset(s_buf, 0 ,sizeof(s_buf));	
			
			if((n = recv(connfd , r_buf , sizeof(r_buf), 0 )) < 0)
				perror("Receive error\n");
			r_buf[n] = '\0';
			
			// Seperate username and password

			char *token,*token1;
			token = strtok(r_buf , ",");
			strncpy(entry.username,token,sizeof(entry.username));
			token1= strtok(NULL , ",");
			strncpy(entry.password,token1,sizeof(entry.password));
			
			//check if entry is valid entry
			
			is_entry_valid = check_data_entry(ue_p,entry);
			
			//check if entry is online
			is_entry_online = check_online_entry(ce_p,entry);
			if(is_entry_online)
				flag1 =2;
			if(!is_entry_valid) 
				flag1 = 1;
			}
			while(is_entry_online == 1 || is_entry_valid == 0 );
			
			//Entry is valid and it is now online--------------------------------

			if(is_entry_valid == 1)
			{
			strcpy(s_buf,"----------------Sucessfully logged in-----------------\n");
			if(send(connfd , s_buf , sizeof(s_buf), 0) < 0)
				perror("Send error\n");
			memset(s_buf, 0 ,sizeof(s_buf));
			}

			strcpy(connection.username , entry.username);
			connection.sockfd = connfd;
			
			//Add the new entry to online connections

			create_conn_entry(ce_p,connection);
			printf("\n\n--------Online connections---------\n\n");
			list_online_connections(ce_p);
			printf("\n\n\n");
			for (int i = 0; i < max_clients; i++)
			{
				if(client_socket[i] == 0){
					client_socket[i] = connfd;
					break;
				}
			}
		}

		//If client is set
		for (int i = 0; i < max_clients; ++i)
		{
			memset(s_buf, 0 ,sizeof(s_buf));
			memset(r_buf, 0 ,sizeof(r_buf));
			int valread;
			sd = client_socket[i];
			clilen = sizeof(cliaddr);
			if(FD_ISSET (sd , &readfds)){
				if((valread = read(sd , r_buf , 1024)) == 0){
					//Client wants to terminate the connection
					//Remove entry from online connections
					remove_conn_entry(ce_p , sd);
					
					//Get ip address and port of disconnected client
					getpeername(sd, (struct sockaddr *)&cliaddr, &clilen);
					printf("<-->Host disconnected, IP %s, port %d\n",inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port));
					printf("\n\n--------Updated connections---------\n\n");
					
					//List updated connections
					list_online_connections(ce_p);
						
					//Remove entries from read file descriptors
					FD_CLR(sd,&readfds);
					close(sd);
					client_socket[i] = 0;

				}
				
				else{
					//get message and username
					r_buf[valread-1] = '\0';
					int rd;
					char uname[20];
					int flag=0; 
					char newstr[1000]="";
					char *token = strtok(r_buf, ",");
    				char *token1;
					char token2[4]=" : ";
					char *token3;
					while ((token1 != NULL)||(flag==0))
					{   
	    					if(flag==0)
		    					strcat(newstr,token);
							else
		    				{
		    					strcat(newstr,",");
		    					strcat(newstr,token);
							token = strtok(NULL, ",");
							if(token==NULL)
							{
								token=token1;
								break;
							}
							strcat(newstr,",");
		   					strcat(newstr,token1);
		    				}
						if(flag==0)
							token = strtok(NULL, ",");
						if(token==NULL)
						{
							token=token1;
							break;
						}
                				token1 = strtok(NULL,",");
						flag++;
					}
					
					printf("%s",token);
					printf("%s",newstr);

					strncpy(uname,token,sizeof(uname));
					int len = strlen(token);
					
					rd = check_username(ce_p,uname,sd);
					if((rd) > 0)		
					{	
						//Receiver is online
						struct conn_entry connection;
						rewind(ce_p);
						while(fread(&connection , sizeof(struct conn_entry) , 1 , ce_p) == 1)
							{
								if(sd == connection.sockfd)
									{
					 					token3=connection.username;
										break;
									}
							}	

						strcat(newstr,token2);
						strcat(newstr,token3);
						send(rd , newstr ,strlen(newstr) , 0);
					}
					else if(rd < 0){
						//Sender and receiver are same
						strcpy(s_buf , "------------You have sent the message to yourself----------\n");
						send(sd , s_buf ,strlen(s_buf) , 0);
					}
					else{	
						//receiver is not online
						strcpy(s_buf , "------------User not online...Message not sent-----------\n");
						send(sd , s_buf ,strlen(s_buf) , 0);
					}
				}
			}
		}

	}
}

	

