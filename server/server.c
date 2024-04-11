/*******************************************************************************
* Server receives UDP and TCP packets from clients.                            *
* Execution: ./server_exe {PORTO_TURMAS} {PORTO_CONFIG} {ficheiro configuração}    *
* gcc server/server.c admin/admin.c admin/admin.h user/user.h -Wall -Wextra    *
*  -o server/server_exe                                                        *         
*******************************************************************************/
#include "../admin/admin.h"
#include "../client/client.h"
#include "../user/user.h"
#include "../functions/functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER_LEN      1024
#define PORTO_TURMAS    argv[1]
#define PORTO_CONFIG    argv[2]
#define CONFIG_FILE     argv[3]

void handle_tcp_connection(int tcp_socket);
int handle_udp_connection(int udp_socket);
/*
* Checks if login command was used correctly.
* @param input Login command received from user.
* @return If login is correct returns the user received,
*         otherwise returns null.
*/
struct_user* verify_login_command(char* input, char* message);
/*
* Returns max between x and y.
*/
int max(int x, int y);

int main(int argc, char *argv[]){
    /* Check if all arguments have been given */
    if(argc != 4)
        error("./%s <PORTO_TURMAS> <PORTO_CONFIG> <ficheiro configuração>\n", argv[0]);

    define_config_file(CONFIG_FILE);

    struct sockaddr_in server_udp, server_tcp;
    
    // Preenchimento da socket address structure - udp
	server_udp.sin_family = AF_INET;
	server_udp.sin_port = htons(atoi(PORTO_CONFIG));
	server_udp.sin_addr.s_addr = htonl(INADDR_ANY);

    // Preenchimento da socket address structure - tcp
    bzero((void *) &server_tcp, sizeof(server_tcp));
    server_tcp.sin_family = AF_INET;
    server_tcp.sin_addr.s_addr = htonl(INADDR_ANY);
    server_tcp.sin_port = htons(atoi(PORTO_TURMAS));

    /* Handle TCP */

    /* TCP socket */
    int tcp_socket;
    if((tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) 
        error("Na funcao socket");

    if(bind(tcp_socket, (struct sockaddr*) &server_tcp, sizeof(server_tcp)) < 0) 
        error("Na funcao bind");

    if(listen(tcp_socket, 5) < 0) 
        error("Na funcao listen");
    
    /* Handle UDP */

    /* UDP socket */
    int udp_socket;
	if((udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) 
		error("Erro na criação do socket");

	if(bind(udp_socket, (struct sockaddr*) &server_udp, sizeof(server_udp)) == -1)
		error("Erro no bind");
    
    fd_set rset;
    FD_ZERO(&rset);

    int max_socket = max(tcp_socket, udp_socket) + 1; 

    while(1){
        FD_SET(tcp_socket, &rset);
        FD_SET(udp_socket, &rset);

        int nready = select(max_socket, &rset, NULL, NULL, NULL); 

        if(FD_ISSET(tcp_socket, &rset))
            handle_tcp_connection(tcp_socket);

        if(FD_ISSET(udp_socket, &rset)) 
            if(handle_udp_connection(udp_socket) == -1){
                close(udp_socket);
            }
    }
    close(tcp_socket);
    return 0;
}

void handle_tcp_connection(int tcp_socket){
    char client_buffer[BUFFER_LEN];

    struct sockaddr_in client;
    int client_addr_size = sizeof(client), nread;

    //clean finished child processes, avoiding zombies
    //must use WNOHANG or would block whenever a child process was working
    while(waitpid(-1, NULL, WNOHANG) > 0);

    int client_id = accept(tcp_socket, (struct sockaddr *) &client, (socklen_t *) &client_addr_size);
    if(client_id > 0){
        if(fork() == 0){
            close(tcp_socket);
            /*
            *   Verify login
            */
            nread = read(client_id, client_buffer, BUFFER_LEN-1);
            client_buffer[nread] = '\0';

            fflush(stdout);

            printf("TCP: %s\n", client_buffer);

            char* message = (char*) malloc(BUFFER_LEN * sizeof(char));

            /* Struct with info from user */
            struct_user* user = verify_login_command(client_buffer, message);
            if(user != NULL){
                if(verify_login(user, "TCP")){
                    strcpy(message, "OK");
                    write(client_id, message, strlen(message));
                    while(1){
                        nread = read(client_id, client_buffer, BUFFER_LEN-1);
                        client_buffer[nread] = '\0';
                        
                        printf("TCP: %s\n", client_buffer);

                        int temp = client_verify_command(client_buffer, message);
                        if(temp == -1){
                            write(client_id, message, strlen(message));
                        } else if(temp == 0){
                            write(client_id, message, strlen(message));
                        } else{
                            if(strcmp(message, "LIST_CLASSES") == 0){
                                strcpy(message, "From server: LIST_CLASSES");
                                write(client_id, message, strlen(message));
                                // POSTERIORMENTE CHAMA-SE A FUNCAO RESPONSÁVEL
                            } else if(strcmp(message, "LIST_SUBSCRIBED") == 0){
                                strcpy(message, "From server: LIST_SUBSCRIBED");
                                write(client_id, message, strlen(message));
                                // POSTERIORMENTE CHAMA-SE A FUNCAO RESPONSÁVEL
                            } else if(strcmp(message, "SUBSCRIBE_CLASS") == 0){
                                strcpy(message, "From server: SUBSCRIBE_CLASS");
                                write(client_id, message, strlen(message));
                                // POSTERIORMENTE CHAMA-SE A FUNCAO RESPONSÁVEL
                            } else if(strcmp(message, "CREATE_CLASS") == 0){
                                strcpy(message, "From server: CREATE_CLASS");
                                write(client_id, message, strlen(message));
                                // POSTERIORMENTE CHAMA-SE A FUNCAO RESPONSÁVEL
                            } else if(strcmp(message, "SEND") == 0){
                                strcpy(message, "From server: SEND");
                                write(client_id, message, strlen(message));
                                // POSTERIORMENTE CHAMA-SE A FUNCAO RESPONSÁVEL
                            }
                        }
                    }
                } else{
                    strcpy(message, "REJECTED\n");
                    write(client_id, message, strlen(message));
                }
            } else
                write(client_id, message, strlen(message));

            close(client_id);
            exit(0);
        }
        close(client_id);
    }
}

int handle_udp_connection(int udp_socket){
    struct sockaddr_in admin;

    /* Save messages from client */
	char admin_buffer[BUFFER_LEN];
    int recv_len;
    socklen_t udp_socket_len = sizeof(admin); 

    if((recv_len = recvfrom(udp_socket, admin_buffer, BUFFER_LEN-1, 0, (struct sockaddr *) &admin, (socklen_t *) &udp_socket_len)) == -1)
        error("Erro no recvfrom");

    // Para ignorar o restante conteúdo (anterior do buffer)
    admin_buffer[recv_len] = '\0';

    printf("UDP: %s", admin_buffer);
    
    char* message = (char*) malloc(BUFFER_LEN * sizeof(char));

    /* Struct with info from user */
    struct_user* user = verify_login_command(admin_buffer, message);
    if(user != NULL){
        if(verify_login(user, "UDP")){
            strcpy(message, "OK\n");
            sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *) &admin, udp_socket_len);

            while(1){
                recv_len = recvfrom(udp_socket, admin_buffer, BUFFER_LEN - 1, 0, (struct sockaddr *) &admin, &udp_socket_len);
                if(recv_len == -1)
                    error("Erro no recvfrom");

                admin_buffer[recv_len] = '\0';
                printf("UDP: %s", admin_buffer);
                
                int temp = verify_admin_command(admin_buffer,message);
                if(temp == -1){
                    sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *) &admin, udp_socket_len);
                } else if(temp == 0){
                    sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *) &admin, udp_socket_len);
                } else {
                    if(strcmp(message,"DEL\n") == 0){
                        strcpy(message,"From server: DEL\n");
                        sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *) &admin, udp_socket_len);
                        //POSTERIORMENTE CHAMA-SE A FUNÇÃO RESPONSÁVEL
                    } else if(strcmp(message,"ADD_USER\n") == 0){
                        strcpy(message,"From server: ADD_USER\n");
                        sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *) &admin, udp_socket_len);
                        //POSTERIORMENTE CHAMA-SE A FUNÇÃO RESPONSÁVEL
                    } else if(strcmp(message, "LIST\n") == 0){
                        strcpy(message,"From server: LIST\n");
                        sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *) &admin, udp_socket_len);
                        //POSTERIORMENTE CHAMA-SE A FUNÇÃO RESPONSÁVEL
                    } if(strcmp(message, "QUIT_SERVER\n") == 0){
                        return -1;
                    }
                }
            }
        } else{
            strcpy(message, "REJECTED\n");
            sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *) &admin, udp_socket_len);
        }
    } else{
         sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *) &admin, udp_socket_len);
    }
    return 0;
}

struct_user* verify_login_command(char* input, char* message){
    struct_user* user = (struct_user*) malloc(sizeof(struct_user));
    if(user == NULL)
        error("Erro a alocar memória!\n"); 
    
    char command[10];
    char* token = strtok(input, " ");

    if(strcmp(token, "LOGIN") != 0)
        return NULL;

    strcpy(command, token); // command -> "LOGIN"

    token = strtok(NULL, " ");
    if(token == NULL){
        strcpy(message, "LOGIN <username> <password>");
        return NULL;
    }
    strcpy(user->username, token);
    
    token = strtok(NULL, " ");
    if(token == NULL){
        strcpy(message, "LOGIN <username> <password>");
        return NULL;
    }
    strcpy(user->password, token);
    remove_line_break(user->password);

    return user;
}

int max(int x, int y){ 
    if(x > y) 
        return x; 
    return y; 
}