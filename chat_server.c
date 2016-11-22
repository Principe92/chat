#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUFSIZE 100

void * clnt_connection(void *arg);
void send_message(char *message, int len);
void error_handling(char *message);

int clnt_number = 0;
int clnt_socks[10];
pthread_mutex_t mutx;

int main(int argc, char **argv)
{
  int	serv_sock;
  int	clnt_sock;
  int	clnt_addr_size;
  int option = 1;
  struct	sockaddr_in serv_addr;
  struct 	sockaddr_in clnt_addr;
  pthread_t	thread;
  
  printf("============================\n");
  printf("WELCOME TO THE CHAT - SERVER\n");
  printf("============================\n");
  
  if(argc != 2){
    printf ("Usage : %s <port>\n", argv[0]);
    exit(1);
  }
  
  if(pthread_mutex_init(&mutx, NULL))
    error_handling("mutex init error");
  
  //create the server socket
  serv_sock = socket(PF_INET, SOCK_STREAM, 0);
  if(serv_sock == -1)
    error_handling("socket() error");
  setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
  
  
  //initialize the address info structure variables
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(atoi(argv[1]));
  
  
  //assign the address to the sockets
  if(bind(serv_sock, (struct sockaddr*) &serv_addr,
    sizeof(serv_addr)) == -1)
      error_handling("bind() error");
  
  //enter the state of waiting for the connection request and set the queue size 5 
  if(listen(serv_sock, 5) == -1) 
    error_handling("listen() error");
  
  while(1) {
    //accept the client's connection request
    clnt_addr_size = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct 
      sockaddr*) &clnt_addr, &clnt_addr_size);
    
    //Access the global variable and the array to save the information of the new socket 
    pthread_mutex_lock(&mutx);
    clnt_socks[clnt_number++] = clnt_sock;
    pthread_mutex_unlock(&mutx);
    
    //Create the thread which would specilize on handling I/O of the new socket
    pthread_create(&thread, NULL, clnt_connection,
		   (void *)(intptr_t) clnt_sock);
    printf(" IP : %s \n", inet_ntoa(clnt_addr.sin_addr));
  }
  return 0;
}

//when the socket is shut down the information of the closed socket is exterminated from the entire socket's information
void * clnt_connection(void *arg)
{
  int clnt_sock = *((int *) arg);
  int str_len = 0;
  char message[BUFSIZE];
  int i;
  
  while((str_len = read(clnt_sock,message, sizeof(message)))!= 0)
    send_message(message,str_len);
  
  pthread_mutex_lock(&mutx);
  for(i = 0; i < clnt_number; i++){
    if(clnt_sock == clnt_socks[i]){
      for(;i < clnt_number - 1; i++) clnt_socks[i] = clnt_socks[i + 1];
      break;
    }
  }
  clnt_number--;
  pthread_mutex_unlock(&mutx);
  
  close(clnt_sock);
  return 0;
}

//sends the message to every socket connected to the server
void send_message(char * message, int len)
{
  int i;
  message[len] = 0;
  fputs(message,stdout);
  pthread_mutex_lock(&mutx);
  for(i = 0; i < clnt_number; i++)
    write(clnt_socks[i], message, len);
  pthread_mutex_unlock(&mutx);
  bzero(message, sizeof(message));
}

void error_handling(char *message)
{
  fputs(message,stderr);
  fputc('\n',stderr);
  exit(1);
}