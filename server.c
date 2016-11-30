#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include <stdbool.h>

#define MY_SOCK_PATH "/somepath"
#define LISTEN_BACKLOG 50
#define BUFFER_SIZE 256
#define NAMESIZE 32
#define handle_error(msg) \
   do { perror(msg); exit(EXIT_FAILURE); } while (0)

typedef struct client {
	struct client *next;
	char *name;
	int cfd;
} Client;

void exitChat(Client *); // exit from chat
void *ignore();

Client *head = NULL;
int counter;
char *quit = "quit";
char *name = "name";

void *ignore(){}

char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1)+strlen(s2)+1);
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

Client *addClient(int cfd){
	if (head == NULL){
		head = malloc(sizeof(Client));
		head->name = malloc(NAMESIZE);
		snprintf(head->name, NAMESIZE, "User %d", counter++);
		head->cfd = cfd;
		head->next = NULL;

		return head;
	}else{

		Client *pt = head;
		while (pt->next != NULL) pt = pt->next;

		Client *next = malloc(sizeof(Client));
		next->name = malloc(NAMESIZE);
		snprintf(next->name, NAMESIZE, "User %d", counter++);
		next->cfd = cfd;
		next->next = NULL;
		
		pt->next = next;

		return next;
	}
}

Client *removeClient (int cfd){
	if (head != NULL){
		Client *pt = head;
		Client *prev = NULL;

		while(pt != NULL && pt->cfd != cfd){
			prev = pt;
			pt = pt->next;
		}

		if (pt == NULL){ 
			 return NULL; 		// client not found
		}

		if (prev == NULL){ 		// client is the head
			head = pt->next;
		}

		else if (pt->next == NULL){ 	// client is the tail
			prev->next = NULL;
		}

		else{
			prev->next = pt->next;
		}

		return pt;
	}

	return NULL;
}

void tryWrite(char *msg, Client *pt){
	if (pt != NULL){
		int res = write(pt->cfd, msg, strlen(msg)+1);
		if (res == -1){
			//exitChat(pt);
		}
	}
}

void writeMessage(char *msg, char *msg2, Client *ct){
	Client *pt = head;
	Client *tmp;
	bool res = false;

	Client *fds[BUFFER_SIZE] = {};
	int index = 0;

	while (pt != NULL){
		tmp = pt->next;

		if (pt->cfd != ct->cfd){			
			tryWrite(msg, pt);
		}

		pt = tmp;
	}

	printf("%s", msg);
}

void exitChat(Client *pt){
	char *msg = malloc(BUFFER_SIZE);
	sprintf(msg, "\n%s has disconnected\n", pt->name);
	writeMessage(msg, NULL, pt);

	shutdown(pt->cfd, SHUT_RDWR);
	removeClient(pt->cfd);
	free(pt);
}

int hasName(char *buffer){
	if (strlen(buffer) >= 4){
		return !strncmp(buffer, name, strlen(name));
	}

	return 0;
}

void changeName(Client *ct, char *buffer){
	int len = strlen(buffer);
	char *msg = malloc(BUFFER_SIZE);
	char *name;

	if (len > 5 && buffer[4] == ' '){
		char temp[len - 4];
                int i = 5;
		int k = 0;
                for (i; i < len-1; i++){
			if (buffer[i] != ' ')
			 	temp[k++] = buffer[i];
		}

		if (k == 0){
			sprintf(msg, "Invalid name\n");
			tryWrite(msg, ct);
		} else{
                	temp[k] = '\0';

			sprintf(msg, "%s has changed name to: \"%s\"\n", ct->name, temp);
			writeMessage(msg, NULL, ct);
		

			for (k; k > -1; k--)ct->name[k] = temp[k];
		}
	} else{
                sprintf(msg, "Invalid name\n");
		tryWrite(msg, ct);
	}		
}

void readPort(){
	Client *pt = head;
	char buffer[BUFFER_SIZE];
	bool delete = false;
	Client *tmp = NULL;
 
	while (pt != NULL){
		
		// read each port
		tmp = pt->next;

		while (1){
			int size = read(pt->cfd, buffer, BUFFER_SIZE);
			if (size == 0) {exitChat(pt); break;}
			else if (size < 0 ) break;
			else{

				// check if quit
				if (!strncmp(buffer, quit, strlen(quit))){
					exitChat(pt);
					break;
				}

				else if (hasName(buffer))
					changeName(pt, buffer);

				else{
					char *beg = concat("", pt->name);
					beg = concat(beg, ": ");
					beg = concat(beg, buffer);
					//beg = concat(beg, "\n"); 

					char *name = concat(pt->name, ":");
					writeMessage(beg, name, pt);
				}
			}

			bzero(buffer, BUFFER_SIZE);
		}

                pt = tmp;

	}
}

int main(int argc, char *argv[]){

	// check for arguments
	if (argc != 3){
		printf("Usage: ./server <IP address> <Port number>\n");
		return 0;
	}

	signal(SIGPIPE, (void *)ignore);

	int port = atoi(argv[2]);
	
	int sfd, cfd;
	int yes = 1;
	char text[BUFFER_SIZE];
	char *msg = malloc(BUFFER_SIZE);
	char *msg2 = malloc(BUFFER_SIZE);
        struct sockaddr_in my_addr, peer_addr;
        socklen_t peer_addr_size;

        sfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (sfd == -1) handle_error("socket");

	// reuse socket
	if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &yes , sizeof(int)) < 0)
    		error("setsockopt(SO_REUSEADDR) failed");

        memset(&my_addr, 0, sizeof(struct sockaddr_in));
        my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port);

	if (inet_aton(argv[1], (struct in_addr *) &my_addr.sin_addr) == 0)
		 handle_error("Unable to convert host address to binary");
	
	// Bind to port
        if (bind(sfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr_in)) == -1)
             handle_error("bind");

	// Now listen for connection
        if (listen(sfd, LISTEN_BACKLOG) == -1)
             handle_error("listen");

	
        peer_addr_size = sizeof(struct sockaddr_in);

	while (1){

		readPort();

         	cfd = accept4(sfd, (struct sockaddr *) &peer_addr, &peer_addr_size, SOCK_NONBLOCK);
		if (cfd != -1){
			
			Client *new = addClient(cfd);

			if (new != NULL){
				sprintf(msg, "%s has joined the chat room\n", new->name);
				sprintf(msg2, "You joined the chat as: \"%s\"\n", new->name);
				writeMessage(msg, msg2, new);

				tryWrite(msg2, new);
			}
		}

		//readPort();

	}

	unlink();
}

