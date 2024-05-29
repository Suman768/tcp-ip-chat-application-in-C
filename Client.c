// INCLUDE ALL THE REQUIRED HEADER LIBRARIES------------------------
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include <ctype.h>

// DEFINING THE NECESSARY MACRO---------------------------
#define SERVER_PORT 5000
#define MAX_LENGTH 2000

// FUNCTION FOR CHECKING TO SUCCESSFUL OPERATIONS ON THE SOCKET-----------------------
bool check(int arg){
	if(arg>=0)
		return true;
	else
		return false;
}

// CONTINUOUSLY CHECKING FOR SERVER MESSAGE------------------------------
void * receiveMessageProcess(void* socketID){
	int server_fd = *((int*) socketID);
	char RECV_MSG[MAX_LENGTH] ;
	
	while(1){

		// memset() is used to fill a block of memory with a particular value.
		// FILL WITH NULL
		memset(RECV_MSG,'\0',sizeof(RECV_MSG));
		
		// RECEIVE THE SERVER RESPONSE
		if(recv(server_fd, RECV_MSG, sizeof(RECV_MSG), 0) < 0){
			printf("\033[1;31m[ERROR] Server Message Not Received \033[0m\n");
			return -1;
		}
		
		// DISPLAY THE SERVER'S MESSAGE
		printf("%s\n",RECV_MSG);
	}
}

// MAIN FUNCTION-----------------------------------------
int main(int argc, char const* argv[]){
    // int valread;
	// char ch[20];

	char SEND_MSG[MAX_LENGTH];

	// TCP SOCKET CREATION
	int socket_descriptor=socket(AF_INET,SOCK_STREAM,0);
	if(check(socket_descriptor)){
		printf("\n\t\t\t🆗 \033[1;32m Socket Created Successfully | SOCKET ID : %d\033[0m ✅\n",socket_descriptor);
	}
	else{
		perror("❌ \033[31m Socket Creation Failed \033[0m❌");
		printf(" ");
	}

	// GIVE THE SERVER's IP ADDRESS AND PORT TO CONNECT THE SERVER------------
	struct sockaddr_in server_address;
	
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(SERVER_PORT);
	server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	// CONNECTING TO SERVER
	int status = connect(socket_descriptor, (struct sockaddr*)&server_address, sizeof(server_address));
	if(status>=0){
        printf("\t\t\t🆗 \033[1;33m Client Connected to the Server PORT--> %d\033[0m ✅ \n\n",SERVER_PORT);

	}
	else{
		// printf("%d",status);
		perror("\t\t\t❌ \033[31m Client Can't Connect to the Server \033[0m❌\n");
		printf(" ");
	}

	//CREATING CLIENT THREAD
	pthread_t thread;
	pthread_create(&thread, NULL, receiveMessageProcess, (void*) &socket_descriptor);
	
	printf("\033[1;92m \n\n+++++++++++++++ CONNECTED TO TCP SERVER +++++++++++++++++ \033[0m\n");
	
    while(1){
		// EMPTY THE SEND MSG BUFFER
		bzero(SEND_MSG, sizeof(SEND_MSG));
		
		// GETTING INPUT FROM CLIENT
		gets(SEND_MSG);
		
		// IF CLIENT WANT TO 'EXIT'
		if(strcmp(SEND_MSG, "EXIT")==0){
			
			if(send(socket_descriptor, SEND_MSG, strlen(SEND_MSG), 0) < 0){
				printf("\033[1;31m[ERROR] 'EXIT' Msg Not Send to Server\033[0m\n");
				return -1;
			}
			printf("\033[1;31m \n\n[EXIT SUCCESSFULLY]\033[0m\n");
			break;
		}

		// IF CLIENT WANT TO COMMUNICATE
		else{
			// Send the message to server:
			if(send(socket_descriptor, SEND_MSG, strlen(SEND_MSG), 0) < 0){
				printf("\033[1;31m[ERROR] Msg Not Send to Server\033[0m\n");
				return -1;
			}
		}
    }
	
    // CLOSE THE CLIENT SOCKET
    close(socket_descriptor);
	printf("\033[1;31m[CONNECTION TERMINATED]\033[0m\n\n");
	
    return 0;

}