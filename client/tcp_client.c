// João Afonso dos Santos Simões - 2022236316
// Rodrigo Miguel Santos Rodrigues - 2022233032
#include "../functions/functions.h"
#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/select.h>

/*------------------------------------------------------------------------------------------------------*
*                                           TCP ADMIN INTERFACE                                         *
*********************************************************************************************************
*   This program is a client that connects to the server using the UDP protocol.                        *
*   The client send commands to the server and receive responses.                                       *
*   Command Syntax:                                                                                     *
*       - class_client {endereço do servidor} {PORTO_TURMAS}                                            *
*********************************************************************************************************/
int main(int argc, char* argv[]){
    signal(SIGINT, handle_sigint); // Handle SIGINT signal
    
    father_pid = getpid();
    
    /*  
    *   Verify client execution
    */
    if(argc != 3){
        error("Bad run! Try: %s <host> <port>", argv[0]);
    }


    /*
    *   Socket creation and connection to the server.
    */

    struct sockaddr_in addr;
    struct hostent *hostPtr;

    if((hostPtr = gethostbyname(argv[1])) == 0){
        cleanup_resources();
        error("Cannot get host address: ");
    }

    bzero((void *) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ((struct in_addr *) (hostPtr->h_addr))->s_addr;
    addr.sin_port = htons((short) atoi(argv[2]));

    if((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
        cleanup_resources();
        error("TCP socket: ");
    }
    
    if(connect(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0){
        cleanup_resources();
        error("TCP connection to the server: ");
    }


        
    /*
    *   Command Line Interface
    */
    int nread = 0; // Number of bytes read.

    buffer = (char*) malloc(BUFFER_LEN * sizeof(char));
    if(buffer == NULL){
        cleanup_resources();
        error("buffer -> malloc(): ");
    }

    input = (char*) malloc(INPUT_SIZE * sizeof(char));
    if(input == NULL){
        cleanup_resources();
        error("input -> malloc(): ");
    }

    printf("Connection established\n");

    /*
    *   Command Line Interface
    */
    while(1){
        printf(">> ");

        // send message
        if(fgets(input, INPUT_SIZE - 1, stdin) == NULL){
            cleanup_resources();
            error("Erro a ler o comando");
        }
        remove_line_break(input);

        write(fd, input, strlen(input));

        // Message received from server
        nread = read(fd, buffer, BUFFER_LEN);
        if(nread < 0)
            continue;
        if(nread == 0){
            printf("CONNECTION CLOSED\n");
            break;
        }
        
        buffer[nread] = '\0';

        printf("%s\n", buffer);

        /*
        *  Distinguish "OK" from result of LOGIN and OK from others commands.
        */
       
        if(!logged_in){
            char* token = strtok(buffer, " ");
            if(strcmp(token, "OK") == 0){
                logged_in = 1;
            } else if(strcmp(buffer, "REJECTED") == 0){
                cleanup_resources();
                exit(0);
            } else if(strcmp(token, "LOGIN") == 0){
                cleanup_resources();
                exit(0);
            }
        }


        /*
        *   User subscribed a class.
        */
       
        char* token = strtok(buffer, " ");
        if(strcmp(token, "ACCEPTED") == 0){
            token = strtok(NULL, " ");
            if(token != NULL){
                /*
                *   Join the multicast group.
                */
               
                // create a UDP socket
                if((multicast_sock[multicast_index] = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
                    cleanup_resources();
                    error("Multicast socket(): ");
                }

                int reuse = 1;
                if(setsockopt(multicast_sock[multicast_index], SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0){
                    close(multicast_sock[multicast_index]);
                    error("Multicast setsockopt(SO_REUSEADDR)");
                }

                // set up the multicast address structure
                struct sockaddr_in multicast_addr;   // Multicast address.
                memset(&multicast_addr, 0, sizeof(multicast_addr));
                multicast_addr.sin_family = AF_INET;
                multicast_addr.sin_addr.s_addr = htonl(INADDR_ANY);
                multicast_addr.sin_port = htons(last_port_used);
                last_port_used++;

                // bind the socket to the port
                if(bind(multicast_sock[multicast_index], (struct sockaddr *)&multicast_addr, sizeof(multicast_addr)) < 0){
                    error("Multicast bind(): ");
                }

                // join the multicast group
                struct ip_mreq mreq;
                mreq.imr_multiaddr.s_addr = inet_addr(token);
                mreq.imr_interface.s_addr = htonl(INADDR_ANY);
                if(setsockopt(multicast_sock[multicast_index], IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0){
                    cleanup_resources();
                    error("Multicast setsockopt(): ");
                }

                multicast_mreq[multicast_index] = mreq;

                // Creates the process responsible for reading the multicast messages
                if((multicast_pid[multicast_index] = fork()) == 0){
                    read_multicast_messages(multicast_index);
                    exit(0);
                } else if(multicast_pid[multicast_index] < 0){
                    cleanup_resources();
                    error("Error creating process to read the multicast messages: ");
                }
                multicast_index++;
            }
        }
    }
    cleanup_resources();
    return 0;
}

// tcp_client.c