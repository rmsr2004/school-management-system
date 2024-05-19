// João Afonso dos Santos Simões - 2022236316
// Rodrigo Miguel Santos Rodrigues - 2022233032
#include "../functions/functions.h"
#include "../admin/admin.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>

/*------------------------------------------------------------------------------------------------------*
*                                           UDP ADMIN INTERFACE                                         *
*********************************************************************************************************
*   This program is a client that connects to the server using the UDP protocol.                        *
*   The client send commands to the server and receive responses.                                       *
*   Command Syntax:                                                                                     *
*       - class_admin {endereço do servidor} {PORTO_CONFIG}                                             *
*********************************************************************************************************/
int main(int argc, char* argv[]){
    signal(SIGINT, sigint_handler); // Handle SIGINT signal

    if(argc != 3){
        error("Usage: %s <host> <port> \n", argv[0]);
    }

    // Socket creation
    if((udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
        error("Error creating socket");
    }

    /* 
    *   Set up the server 
    */
    server.sin_family      = AF_INET;                      // Internet Domain  
    server.sin_port        = htons(atoi(argv[2]));         // Server Port       
    server.sin_addr.s_addr = inet_addr(argv[1]);           // Server's Address  
    socklen_t slen = sizeof(server);
    
    input = (char*) malloc(INPUT_SIZE * sizeof(char)); 
    if(input == NULL){
        error("Error malloc");
    }

    buffer = (char*) malloc(BUFFER_LEN * sizeof(char));
    if(buffer == NULL){
        error("Error malloc");
    }

        
    /*
    *	Avoid the problem of the first message not being sent on GNS3
    */
    
    sprintf(input, "CONNECTION");
    
    if(sendto(udp_socket, input, (strlen(input)+1), 0, (struct sockaddr *)&server, sizeof(server)) < 0){
        error("sendto()");
    }
    if(sendto(udp_socket, input, (strlen(input)+1), 0, (struct sockaddr *)&server, sizeof(server)) < 0){
        error("sendto()");
    }
    if(sendto(udp_socket, input, (strlen(input)+1), 0, (struct sockaddr *)&server, sizeof(server)) < 0){
        error("sendto()");
    }

    printf("Connection established\n");

    /*
    *   Command Line Interface
    */   
    while(1){
        printf(">> ");

        if(fgets(input, INPUT_SIZE-1, stdin) == NULL){
            error("Error fgets()");
        }
        remove_line_break(input);

        // Send message to server
        if(sendto(udp_socket, input, (strlen(input)+1), 0, (struct sockaddr *)&server, sizeof(server)) < 0){
            error("sendto()");
        }
        
        // Receive message from server
        if((recv_len = recvfrom(udp_socket, buffer, BUFFER_LEN, 0, (struct sockaddr *) &server, &slen)) == -1){
	        error("Erro no recvfrom");
        }
        buffer[recv_len] = '\0';

        if(strcmp(buffer, "QUIT_SERVER\n") == 0){
            server_closing();
            exit(0);
        }

        printf("%s", buffer);

        // if message received is REJECTED or LOGIN <username> <password> break the loop
        if(strcmp(buffer, "REJECTED\n") == 0 || strcmp(buffer, "LOGIN <username> <password>") == 0)
            break;
    }
    printf("\n");
    close(udp_socket);  // Deallocate the socket
    return 0;
}

// udp_admin.c