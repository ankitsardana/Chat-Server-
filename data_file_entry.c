#include<netinet/in.h>			//for defining the socket address structure
#include<sys/socket.h>			//for defining the generic socket address structure
#include<arpa/inet.h>			//IP address conversion stuff		
#include<unistd.h>			//defines STDIN_FILENO, system calls,etc
#include<string.h>			//for handling the string related function.
#include<stdio.h>			//standard C i/o facilities
#include<stdlib.h>			//for handling the standard library functions
#include<errno.h>			//it defines macros for reporting and retrieving error conditions


struct user_entry{
	char username[20];
	char password[8];
};
void create_data_entry(FILE *fp , struct user_entry entry){
	/*
	Objective : Store a registered entry in a file
	Input parameters : fp - file pointer of file containing online connection entries
					   entry - filled connected entry  
	*/
	fseek(fp , 0 ,SEEK_END);
	fwrite(&entry, sizeof(struct user_entry) , 1 , fp);
}
int check_data_entry(FILE *fp , struct user_entry entry){
	/*
	Objective : Check if a registered entry already exists
	Input parameters : fp - file pointer of file containing online connection entries
					   entry : Filled in registered entry
	Return type : 0 : Entry exists
				  1 : Entry does not exist	   
	*/
	
	rewind(fp);
	struct user_entry connection;
	while(fread(&connection , sizeof(struct user_entry) , 1 , fp) == 1){
		if(strcmp(entry.username , connection.username) == 0)
			return 0;
	}
	return 1;
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
	
	
	FILE *ue_p;

	if((ue_p = fopen("User_data.dat","wb+")) == NULL ){
		printf("Cannot open file");
	} 
	printf("plzz enter username and there passwords for storage : \n");
	while(1)
	{
		fgets(r_buf, 1024, stdin);
		if (strcmp(r_buf , "quit\n") == 0) 
		{
			fclose(ue_p);
			exit(0);
		}
        	else
		{
			char *token,*token1;
			token = strtok(r_buf , ",");
			strncpy(entry.username,token,sizeof(entry.username));
			printf("Username :- %s",entry.username);
			token1= strtok(NULL , ",");
			strncpy(entry.password,token1,sizeof(entry.password));
			printf("          Password :- %s\n",entry.password);
			if(check_data_entry(ue_p,entry)){
				create_data_entry(ue_p,entry);
			}
			else{
				printf("\nUsername already exists\n");
			}

		}
	}
}
