// INCLUDE ALL THE REQUIRED HEADER LIBRARIES------------------------
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include<unistd.h>

// DEFINING THE NECESSARY MACRO---------------------------
#define SERVER_PORT 5000
#define QUEUE_LIMIT 0
#define TOTAL_CLIENT 5
#define MAX_IP_LENGHT 100
#define MAX 2000
#define ACTIVE "\033[1;92mACTIVE"
#define DE_ACTIVE "\033[1;31mDE-ACTIVE"

// DECLARING GLOBAL VARIABLE
static int MAX_ACTIVE_CLIENT;
static int client_count=0, active_client_count=0;

// pthread_mutex_t LOCK FOR CRITICAL SECTION CONTROL------------------------
pthread_mutex_t lock;

// CLIENT STRUCTURE------------------------------
struct client{
	int client_NO;
	int client_socket_ID;
	char client_IP[MAX_IP_LENGHT+1];
	int client_PORT;
	char Active_status[50];
};

// CLIENT STRUCTURE ARRAY-----------------------------------
struct client Client[TOTAL_CLIENT];

// CLIENT THREAD ARRAY----------------------------------
pthread_t client_thread[TOTAL_CLIENT];

// CLIENT ADDRESS ARRAY-----------------------------
struct sockaddr_in client_addr[10];
int addrlen = sizeof(client_addr);

// FUNCTION FOR CHECKING THE SUCCESSFUL OPERATIONS ON THE SOCKET----------------------------
bool check(int arg){
	if(arg>=0)
		return true;
	else
		return false;
}

/*FUNCTION FOR CHECKING TOTAL CLIENT LIST
---------------------------------------------------------------------------------------------
|	void clientList(void* Clients){															|
|		struct client* ClientDetails=(struct client*)Clients;								|
|		for(int i=0;i<TOTAL_CLIENT;i++){													|
|			printf("Client_No -> %d\n",ClientDetails[i].client_NO);							|
|			printf("Client_Socket_ID -> %d\n",ClientDetails[i].client_socket_ID);			|
|			printf("Client_IP -> %s\n",ClientDetails[i].client_IP);							|
|			printf("Client_PORT -> %d\n",ClientDetails[i].client_PORT);						|
|			printf("Active_status -> %s\n",ClientDetails[i].Active_status);					|
|			printf("------------------------------------------------------------\n\n\n");	|
|		}																					|
|	}																						|
---------------------------------------------------------------------------------------------
*/

// COMMUNICATION SERVICE WHICH IS MAINTAINED BY POSIX THREAD 
void * communicationService(void * currentClient){

	struct client* currentClient_struct = (struct client*) currentClient;
	
	char* SEND_MSG = "HTTP/1.1 200 OK\r\n\r Hello from server";
	char RECV_MSG[2000];

	// GETTING THE CLIENT NO AND CLIENT SOCKET ID
	int clientNo = currentClient_struct -> client_NO;
	int clientSocketID = currentClient_struct -> client_socket_ID;
	
		// CHECKING IF ACTIVE CLIENT IS WITHIN THE LIMIT OF MAXIMUM ACTIVE CLIENT
		if(active_client_count<=MAX_ACTIVE_CLIENT){

			char SEND_MSG[MAX];
			char OPTION_MSG[MAX] = "\n\033[91m\033[107m\t\t\t\t\t----- <CHOOSE AN OPTION> -----\n\n [LIST]\t\t\t        --> To Display the Total Client List and Active Status\n [Client No : Message Content]\t--> To Message The Client\n [EXIT]\t\t\t\t--> To Terminate the connection with The Server\n\033[0m\n";

			bzero(SEND_MSG, sizeof(SEND_MSG));

			strcpy(SEND_MSG, OPTION_MSG);
			
			if(send(clientSocketID, SEND_MSG, strlen(SEND_MSG), 0) < 0){
				printf("\033[1;33m [ERROR] Message Not Send \033[0m\n");
				return NULL;
			}

			// MESSAGE GENERATE
			while(1){

				char RECV_MSG[MAX];
				bool COMMUNICATION = false;
				
				// CLEARING THE MESSAGE BUFFERS
				bzero(RECV_MSG, sizeof(RECV_MSG));
				bzero(SEND_MSG, sizeof(SEND_MSG));
				
				// RECEIVING CLIENT MESSAGE
				if(recv(clientSocketID, RECV_MSG, sizeof(RECV_MSG), 0) < 0){
					printf("\033[1;33m [ERROR] Client Message Not Received \033[0m\n");
					break;
				}
				sleep(1);
				printf("\n [ AN OPTION RECEIVED ] --> CLIENT N0 : %d | CLIENT SOCKET Id : %d | CLIENT OPTION : %s \n\n", clientNo, clientSocketID, RECV_MSG);
				
				//CHECKING IF THE CLIENT MSG CONTAINS COLON ':' THEN SETTING COMMUNICATION = true;
				char *isColon;
				isColon = strstr(RECV_MSG,":");
				if(strlen(&isColon) != 0){
					COMMUNICATION = true;
				}
				
				// IF CLIENT CHOOSE 'EXIT' THEN BREAK THE WHILE LOOP & TERMINATE THE SESSION
				if(strcmp(RECV_MSG, "EXIT") == 0){
					break;
				}

				// IF CLIENT CHOOSE 'LIST' THEN SEND TOTAL CLIENT LIST
				else if(strcmp(RECV_MSG, "LIST") == 0){
					int list_buffer = 0;
					if(client_count == 0){
						list_buffer += sprintf(SEND_MSG + list_buffer, "\033[1;35m[SERVER MSG] --> THERE IS CURRENTLY NO CLIENT AVAILABLE...\n");
					}
					else{
						for(int i = 0; i < client_count; i++){
							if(i != index){
								list_buffer += sprintf(SEND_MSG + list_buffer, "\033[1;96m[SERVER MSG] --> Client No: %d | Client Socket Id: %d | Active Status : %s\n\n",(i+1), Client[i].client_socket_ID, Client[i].Active_status);	
								// NOTE - sprintf returns the total number of characters written excluding the null-character appended at the end of the string, otherwise a negative number is returned in case of failure.			
							}
						}
					}

					// So to get multiple client list we need to increment SEND_MSG to list_buffer byte for getting that location
					sprintf(SEND_MSG + list_buffer, "\033[0m\n");
				}
				else if(COMMUNICATION==true){
					// SEPARATING CLIENT NO
					char *Client_No_Separation;
					char *Client_Msg_Separation;
					Client_No_Separation = strtok(RECV_MSG, ":");
					
					// Client_No_Separation holds in the String before Colon ':' 
					// Checking if that String is Contain a number ?

					// INITIALLY ASSUME THE STRING CONTAINS NUMBER
					bool isNumber = true;
					for(int i = 0; i < strlen(Client_No_Separation); i++){
						if(isdigit(Client_No_Separation[i]) == false){
							isNumber = false;
						}
					}
					
					int DESTINATION_CLIENT_NUMBER ;
					
					if(isNumber){

						// CONVERTING THE CLIENT NO FROM STRING TO INTEGER
						// NOTE - The atoi function in C converts a string of characters to an integer value. 
						DESTINATION_CLIENT_NUMBER = atoi(Client_No_Separation) - 1;
					
						// GETTING THE MSG STRING
						Client_Msg_Separation = strtok(NULL, ":");
						
						//CHECKING THE CLIENT ACTIVE STATUS IS ACTIVE OR NOT, IF NOT THEN CAN'T SEND THE MSG
						if(strcmp(Client[DESTINATION_CLIENT_NUMBER].Active_status ,ACTIVE) == 0 && Client_Msg_Separation){
							
							// SERVER SENDING MSG TO THE DESTINATION CLIENT
							int list_buffer = 0;
							list_buffer = sprintf(SEND_MSG, "\033[1;32m[RECEIVE MESSAGE] Source Client No -> %d> | The Msg -> \033[1;96m%s", clientNo, Client_Msg_Separation);
							
							// GETTING ALL THE MSG TOKENS
							// strtok() stores the pointer in static variable where did you last time left off , so on its 2nd call , when we pass the NULL , strtok() gets the pointer from the static variable .
							char temp_msg[2000];
							strcpy(temp_msg,Client_Msg_Separation);
							Client_Msg_Separation = strtok(NULL, ":");
							while(Client_Msg_Separation != NULL){
								list_buffer += sprintf(SEND_MSG+list_buffer, ":%s\033[0m", Client_Msg_Separation);
								Client_Msg_Separation = strtok(NULL, ":");
							}
							sprintf(SEND_MSG+list_buffer, "\033[0m", Client_Msg_Separation);
							
							if(send(Client[DESTINATION_CLIENT_NUMBER].client_socket_ID, SEND_MSG, strlen(SEND_MSG), 0) < 0){
								printf("\033[1;33m [ERROR] Message Not Send \033[0m\n");
								break;
							}
							
							// SETUP SERVER ACKNOWLEDGEMENT MSG TO SEND TO THE SOURCE CLIENT
							bzero(SEND_MSG, sizeof(SEND_MSG));
							sprintf(SEND_MSG, "\033[1;35m\n[SEND MESSAGE] Destination Client No -> %d | The Msg -> \033[1;96m%s\033[0m\n", DESTINATION_CLIENT_NUMBER+1, temp_msg);
							
							// SERVER DISPLAY SUCCESSFULL MSG TRANSMISSION ACKNOWLEDGEMENT
							printf("\033[91m\033[107m[MSG SEND SUCCESSFULLY] Source Client -> %d | Destination Client -> %d | The Msg -> %s \033[0m\n", clientNo, DESTINATION_CLIENT_NUMBER+1, temp_msg);
						}
						else{
							sprintf(SEND_MSG, "\033[1;31m\n[SEND MSG ERROR] The client %d is Not Active Now\033[0m\n", DESTINATION_CLIENT_NUMBER+1 );
						}
					}
					else{
						strcpy(SEND_MSG, "\033[1;31m\n[SEND MSG ERROR - WRONG CLIENT NO] Try Again & Please Enter Correct Client No\033[0m\n");
					}
					
				}
				else{
					//WRONG OPTION
					strcpy(SEND_MSG, "\033[1;31m[OPTION ERROR] WRONG INPUT!! PLEASE CHOOSE AN VALID OPTION...\033[0m\n");
				}
				
				// SENDING SERVER ACKNOWLEDGEMENT MSG TO THE SOURCE CLIENT
				if(send(clientSocketID, SEND_MSG, strlen(SEND_MSG), 0) < 0){
					printf("\033[1;33m[ACK ERROR] Acknowledgement Not Send\033[0m\n");
					break;
				}
				
			}
			
			// TERMINATING THE SOURCE CLIENT SOCKET & DEACTIVATING THE CLIENT'S ACTIVE STATUS
			stpcpy(currentClient_struct->Active_status,DE_ACTIVE);
			close(clientSocketID);


			printf("\033[1;31m \n++++++++++[Client %d EXITED Successfully]+++++++++++\033[0m\n\n",clientNo);
			pthread_mutex_lock(&lock);
			active_client_count--;
			pthread_mutex_unlock(&lock);
			return NULL;
			

		}
		else{
			SEND_MSG="MAX ACTIVE CLIENT LIMIT CROSS...please try again later...";
			send(clientSocketID, SEND_MSG, strlen(SEND_MSG),0);
			pthread_mutex_lock(&lock);
			client_count--;
			pthread_mutex_unlock(&lock);
			// sleep(10);
			close(clientSocketID);
			printf("closed");
		}
	}
	


// MAIN FUNCTION---------------------------------------------------------------------------
int main(){

	// TCP SOCKET CREATION
	int socket_descriptor=socket(AF_INET,SOCK_STREAM,0);
	if(check(socket_descriptor)){
		printf("\n\t\t\t🆗 \033[1;32m Socket Created Successfully | SOCKET ID : %d\033[0m ✅\n",socket_descriptor);
	}
	else{
		perror("❌ \033[31m Socket Creation Failed \033[0m❌");
		printf(" ");
	}

	// SET UP THE IP ADDRESS AND PORT TO THE SERVER
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(SERVER_PORT);
	server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	// BIND THE SOCKET TO THAT SERVER ADDRESS AND PORT NO.
	if(check(bind(socket_descriptor,(struct sockaddr*)&server_address,sizeof(server_address)))){
		printf("\t\t\t🆗 \033[1;33m Socket Binded to the PORT --> %d         \033[0m ✅ \n",SERVER_PORT);
	}
	else{
		perror("\t\t\t❌ \033[31m Socket binding failed \033[0m❌\n");
		printf(" ");
	}

	// SOCKET IS OPEN TO LISTEN FOR INCOMING REQUEST
	if (check(listen(socket_descriptor, QUEUE_LIMIT))){
		printf("\t\t\t🆗 \033[1;36m Socket is Listening to the PORT --> %d   \033[0m ✅ \n\n",SERVER_PORT);
	}
	else{
		perror("\t\t\t❌ \033[31m Socket Listening failed \033[0m❌\n");
		printf(" ");
	}

	// GETTING INPUT FOR MAXIMUM ACTIVE CLIENT
	printf("ENTER THE NUMBER OF MAXIMUM ACTIVE CLIENT --> ");
	scanf("%d",&MAX_ACTIVE_CLIENT);

	// CHECKING IF MAX ACTIVE CLIENT IS GREATER THAN TOTAL CLIENT----------
	while(1){
		if(MAX_ACTIVE_CLIENT>TOTAL_CLIENT){
			printf("----[ NOTE : Maximum Active Client Can't be Greater than Total Client]----\n");
			printf("ENTER THE NUMBER OF MAXIMUM ACTIVE CLIENT --> ");
			scanf("%d",&MAX_ACTIVE_CLIENT);
		}
		else{
			break;
		}
	}

	printf("\n\t\t\t\033[91m\033[107m  TOTAL NO. OF CLIENT THE SERVER CAN HANDLE ----> %d  \033[0m\n\n ",TOTAL_CLIENT);
	printf("\t\t\t\033[91m\033[107m  MAX ACTIVE OF CLIENT THE SERVER CAN HANDLE ----> %d  \033[0m\n\n",MAX_ACTIVE_CLIENT);


	// Accepting Client Requests-----------------------------------------------
	while(client_count<TOTAL_CLIENT){

		// WAITING FOR CLIENT JOIN REQUEST
		printf("\t\t\t\033[1;36m       🆗  Waiting for Client Requests....  ✅\033[0m\n\n");
		Client[client_count].client_socket_ID=accept(socket_descriptor,(struct sockaddr*)&client_addr[client_count],(socklen_t*)&addrlen);

		// SETTING CLIENT NO
		Client[client_count].client_NO = (client_count+1);

		// GETTING CLIENT IP ADDRESS
		inet_ntop(AF_INET, &client_addr[client_count].sin_addr, Client[client_count].client_IP, 100);

		// GETTING CLIENT PORT NUMBER
		Client[client_count].client_PORT = (int) ntohs(client_addr[client_count].sin_port);

		// SETTING CLIENT ACTIVE STATUS
		stpcpy(Client[client_count].Active_status,ACTIVE);

		// SERVER MSG FOR SUCCESSFUL CLIENT JOIN
		printf("\033[1;32m🆗 CONNECTED CLIENT : Client NO -> %d | Client Socket Id -> %d | Client ADD -> %s : %d ✅\033[0m\n\n",Client[client_count].client_NO,Client[client_count].client_socket_ID, Client[client_count].client_IP, Client[client_count].client_PORT);

		// ERROR WHILE ACCEPTING CLIENT REQUEST
		if(Client[client_count].client_socket_ID<0)
			perror("SOCK_ERR");

		// CREATING CLIENT THREAD FOR COMMUNICATION (POSIX THREAD)
		pthread_create(&client_thread[client_count], NULL, communicationService, (void*)&Client[client_count]);

		// CLITICAL SECTION MUTUAL EXCLUSIVE LOCK
		pthread_mutex_lock(&lock);
		client_count++;
		active_client_count++;
		pthread_mutex_unlock(&lock);
	}

	// PROCESS IS WAITING FOR ALL THE ACTIVE THREADS TO COMPLETE THEIR TASKS 
	for(int i = 0; i < client_count; i++){
		pthread_join(client_thread[i], NULL);
	}

	//CLOSING THE SERVER SOCKET
	close(socket_descriptor);
	printf("\n\n---THE SERVER IS CLOSED SUCCESSFULLY---\n\n");

	return 0;

}
