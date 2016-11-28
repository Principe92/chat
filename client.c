#include <stdio.h>
 #include <stdlib.h>
 #include <string.h>          
 #include <unistd.h>          
 #include <sys/types.h>
 #include <sys/socket.h>         
 #include <netinet/in.h>         
 #include <pthread.h>          

 #define MYPORT 2012     /* default port number */
 #define MAXDATALEN 256
 int      sockfd;
 int namez;
 struct sockaddr_in serv_addr;          /* structure to hold server's address */
 char      buffer[MAXDATALEN];
 char      buf[10];
 void* chat_write(int);
 void* chat_read(int);

 int main(int argc, char *argv[]){
  pthread_t thr1,thr2;          /* variable to hold thread ID */
     if( argc != 3 ){
       printf("Usage: ./client <IP address> <Port Number>\n");
       exit(0);
       }

      /*=============socket creating==============*/
      sockfd = socket(AF_INET, SOCK_STREAM, 0);
      if (sockfd == -1)
        printf ("client socket error\n");
      else
        printf("Socket Created\n");

      /*===============set info===================*/
      bzero((char *) &serv_addr, sizeof(serv_addr));
      serv_addr.sin_family = AF_INET;
      serv_addr.sin_port = htons(atoi(argv[2]));
      serv_addr.sin_addr.s_addr = inet_addr(argv[1]);

      /*=============client connect to server============*/
      if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))==-1)
      {
        printf("Client Connect Error\n");
        exit(0);
      }
      else
     	 printf("Connected to Server\n");
    

      pthread_create(&thr2,NULL,(void *)chat_write,(void *)(intptr_t)sockfd);
      pthread_create(&thr1,NULL,(void *)chat_read,(void *)(intptr_t)sockfd);    
      pthread_join(thr2,NULL);
      pthread_join(thr1,NULL);

      return 0;
 }

 void* chat_read(int sockfd)
 {
        while(1){

	      	int  n;
		n  = read(sockfd,buffer,MAXDATALEN-1);
         	
		if(n==0){
           		 printf("\nDUE TO SOME UNEXPECTED REASONS SERVER HAS BEEN SHUTDOWN\n\n");
             		exit(0);
            	 }

	         if(n>0){
			__fpurge(stdout);

        	      	printf("%s",buffer);
            		bzero(buffer,MAXDATALEN);
             	}
     	 }
 }

 
 void* chat_write(int sockfd)
 {
      while(1){
		fgets(buffer,MAXDATALEN-1,stdin);

        	int n;
		n = write(sockfd,buffer,strlen(buffer)+1);

	         if(strncmp(buffer,"quit",4)==0)
        	     exit(0);

        	 bzero(buffer,MAXDATALEN);
      }
 }
