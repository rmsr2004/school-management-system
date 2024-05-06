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

/*
* Handle SIGINT signal.
*/
void handle_sigint(int signum);
/*
* Process that reads multicast messages.
*/
void read_multicast_messages(int index);
/*
* Cleanup resources.
*/
void cleanup_resources();

/* -------------------------------------------------------------------------------------- */                            
/*                              Global Variables                                          */
/* -------------------------------------------------------------------------------------- */
int fd;                                 // TCP socket file descriptor.
int multicast_sock[MAX_USERS];          // Multicast socket file descriptor.
int multicast_index = 0;                // Multicast Process Index.
int logged_in = 0;                      // Flag to check if the user is logged in.
int last_port_used = 5000;              // Last port used to join the multicast group.
struct sockaddr_in multicast_addr;      // Multicast address.
char* buffer;                           // Buffer to store the messages.
char* input;                            // Buffer to store the input.
pid_t pid;                              // Read Message Queue Process ID.
pid_t multicast_pid[MAX_USERS];         // Multicast Process ID.
pid_t father_pid;                       // Father Process ID.
/* -------------------------------------------------------------------------------------- */

/*
*  TCP Client execution: ./open_client.sh
*  If you want to change server addres and port, you should replace the values in the script on line 14:
*  ./client/client <server_ip> <server_port> 
*/ 
int main(int argc, char* argv[]){
    father_pid = getpid();
    
    /*  Verify client execution */
    if(argc != 3){
        error("Bad run! Try: %s <host> <port>", argv[0]);
    }

    signal(SIGINT, handle_sigint); // Handle SIGINT signal

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

    // Accessing to the message queue created by the server
    if((key_msg = ftok(MSQ_QUEUE_PATH, 'A'))== -1){
        cleanup_resources();
        error("Message Queue ftok(): ");
    }

    if((mq_id = msgget(key_msg, IPC_CREAT | 0700)) == -1){
        cleanup_resources();
        error("msgget(): ");
    }

    // Creates the process responsible for reading the message queue
    if((pid = fork()) == 0){
        handle_quit_server();
        exit(0);
    } else if(pid < 0){
        cleanup_resources();
        error("Error creating process to read the message queue: ");
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


    /*
    * Command Line Interface
    */
    while(1){
        /* enviar comando para o servidor */
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
        if(nread == 0)
            break;
        buffer[nread] = '\0';

        printf("%s\n", buffer);

        if(!logged_in){
            char* token = strtok(buffer, " ");
            if(strcmp(token, "OK") == 0){
                logged_in = 1;
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

                // set up the multicast address structure
                memset(&multicast_addr, 0, sizeof(multicast_addr));
                multicast_addr.sin_family = AF_INET;
                multicast_addr.sin_addr.s_addr = htonl(INADDR_ANY);
                multicast_addr.sin_port = htons(last_port_used + 1);
                last_port_used++;

                // bind the socket to the port
                if(bind(multicast_sock[multicast_index], (struct sockaddr *)&multicast_addr, sizeof(multicast_addr)) < 0){
                    error("Multicast bind(): ");
                }

                // join the multicast group
                struct ip_mreq mreq;
                mreq.imr_multiaddr.s_addr = inet_addr(token);
                mreq.imr_interface.s_addr = INADDR_ANY;
                if(setsockopt(multicast_sock[multicast_index], IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0){
                    cleanup_resources();
                    error("Multicast setsockopt(): ");
                }

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

        if(strcmp(buffer, "REJECTED") == 0 || strcmp(buffer, "LOGIN <username> <password>") == 0 || strcmp(buffer, "You are already logged in!") == 0){
            if(!logged_in){
                cleanup_resources();
                exit(0);
            }
        }
    }
    cleanup_resources();
    return 0;
}

void handle_sigint(int signum){
    if(signum == SIGINT){
        if(getpid() == father_pid){
            printf("\n");
            printf("Exiting...\n");
            char* message = malloc(40 * sizeof(char));
            sprintf(message, "TCP_CLIENT[%d] exiting...", getpid());
            write(fd, message, strlen(message));
            cleanup_resources();
            exit(-1);
        } else{              
            printf("\n");
            printf("Multicast process exiting...\n");
            exit(-1);
        }
    }
}

void read_multicast_messages(int index){
    signal(SIGINT, handle_sigint);

    char* msg_from_multicast = (char*) malloc(BUFFER_LEN * sizeof(char));
    if(msg_from_multicast == NULL){
        cleanup_resources();
        error("msg_from_multicast -> malloc(): ");
    }

    while(1){
        struct sockaddr_in sender_addr;
        socklen_t sender_len = sizeof(sender_addr);
        int nbytes;
        if((nbytes = recvfrom(multicast_sock[index], msg_from_multicast, BUFFER_LEN, 0, (struct sockaddr *)&sender_addr, &sender_len)) > 0){
            msg_from_multicast[nbytes] = '\0';
            printf("%s\n", msg_from_multicast);
        }
    }
    close(multicast_sock[index]);
}

void cleanup_resources(){
    if(fd != -1)  close(fd);
    
    if(buffer != NULL) free(buffer);
    if(input != NULL) free(input);

    for(int i = 0; i < multicast_index; i++){
        if(multicast_pid[i] > 0) kill(multicast_pid[i], SIGINT);
        if(multicast_sock[i] != -1){
            close(multicast_sock[i]);
        }
    }
    if(pid > 0) kill(pid, SIGKILL);
}