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
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <assert.h>

#define PORTO_TURMAS    argv[1]
#define PORTO_CONFIG    argv[2]
#define CONFIG_FILE     argv[3]
#define MAX_TCP_CLIENTS 10

/*
* Handles a TCP connection.
* @param client_id New connection file descriptor.
*/
void handle_tcp_connection(int client_id);
/*
* Handles a UDP connection.
* @param udp_socket UDP socket.
*/
void handle_udp_connection(int udp_socket);
/*
* Checks if login command was used correctly.
* @param input Login command received from user.
* @return If login is correct returns the user received,
*         otherwise returns null.
*/
struct_user verify_login_command(char* input, char* message);
/*
* Handles SIGINT signal.
*/
void sigint_handler();
// Array to save all processes id that are responsible to handle TCP connection with MAX_TCP_CLIENTS length.
pid_t tcp_pids[MAX_TCP_CLIENTS];
// Amount of current TCP connections.
int tcp_pids_index = 0;
// UDP connection pid.
pid_t udp_pid;

int main(int argc, char *argv[]){
    signal(SIGINT, sigint_handler);

    /* Check if all arguments have been given */
    if(argc != 4)
        error("./%s <PORTO_TURMAS> <PORTO_CONFIG> <ficheiro configuração>\n", argv[0]);

    define_config_file(CONFIG_FILE);

    struct sockaddr_in server_udp, server_tcp;
    
    struct sockaddr_in client_tcp;
    int client_addr_size = sizeof(client_tcp);
    
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

    // TCP socket
    int tcp_socket;
    if((tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) 
        error("Na funcao socket");

    if(bind(tcp_socket, (struct sockaddr*) &server_tcp, sizeof(server_tcp)) < 0) 
        error("Na funcao bind");

    if(listen(tcp_socket, 5) < 0) 
        error("Na funcao listen");
    
    /* Handle UDP */

    // UDP socket
    int udp_socket;
	if((udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) 
		error("Erro na criação do socket");

	if(bind(udp_socket, (struct sockaddr*) &server_udp, sizeof(server_udp)) == -1)
		error("Erro no bind");

    /* Creation of message queue to communicate with children processes */
    key = ftok(MSQ_QUEUE_PATH, 'A');
    if (key == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    // Try to delete the key if she exists
    mq_id = msgget(key, 0); // Get message queue id.
    if(mq_id != -1){
        if(msgctl(mq_id, IPC_RMID, NULL) == -1){
            error("[%s] LOG - msgctl: ");
        }
        printf("[%s] LOG - Message queue deleted\n", get_time());
    }
    
    mq_id = msgget(key, IPC_CREAT | 0700);
    if(mq_id == -1){
        error("msgget");
    }

    printf("[%s] LOG - Message queue created\n", get_time());

    /* Creates a single process to every time that a TCP connection is received, the process
    *  creates a new process to handle the connection.
    */
    pid_t tcp_pid;
    if((tcp_pid = fork()) == 0){
        while(tcp_pids_index < MAX_TCP_CLIENTS){
            pid_t pid;

            //clean finished child processes, avoiding zombies
            //must use WNOHANG or would block whenever a child process was working
            while(waitpid(-1, NULL, WNOHANG) > 0);

            int client_id = accept(tcp_socket, (struct sockaddr *) &client_tcp, (socklen_t *) &client_addr_size);
            if(client_id > 0){
                if((pid = fork()) == 0){
                    close(tcp_socket);
                    handle_tcp_connection(client_id);
                    exit(0);
                }
                printf("[%s] LOG - New TCP connection received. Process %d created!\n", get_time(), pid);
                tcp_pids[tcp_pids_index] = pid;
                tcp_pids_index++;
                close(client_id);
            }
        }
        exit(0);
    }
    printf("[%s] LOG - Process %d created to handle TCP connections!\n", get_time(), tcp_pid);

    /* Create a process to handle udp connection */
    if((udp_pid = fork()) == 0){
        handle_udp_connection(udp_socket);
        close(udp_socket);
        exit(0);
    } 
    printf("[%s] LOG - Process %d created to handle UDP connections!\n", get_time(), udp_pid);

    // Waiting for TCP processes to finish
    for(int i = 0; i < MAX_TCP_CLIENTS; i++){
        waitpid(tcp_pids[i], 0, 0);
    }
    wait(NULL); // Waiting for UDP process.

    return 0;
}

void handle_tcp_connection(int client_id){
    char client_buffer[BUFFER_LEN];
    int nread;

    /*
    *   Verify login
    */
    nread = read(client_id, client_buffer, BUFFER_LEN-1);
    client_buffer[nread] = '\0';

    fflush(stdout);

    printf("[%s] LOG - Received message from TCP: %s\n", get_time(), client_buffer);

    char* message = (char*) malloc(BUFFER_LEN * sizeof(char));

    /* Struct with info from user */
    struct_user user = verify_login_command(client_buffer, message);
    if(strcmp(user.username, "a") != 0){
        if(verify_login(&user, "TCP")){
            printf("[%s] LOG - User %s logged in!\n", get_time(), user.username);

            sprintf(message, "OK [%s]", user.type);
            write(client_id, message, strlen(message));
            
            while(1){
                nread = read(client_id, client_buffer, BUFFER_LEN-1);
                client_buffer[nread] = '\0';
                
                printf("[%s] LOG - Received message from TCP: %s\n", get_time(), client_buffer);

                int temp = client_verify_command(client_buffer, user, message);
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
            strcpy(message, "REJECTED");
            write(client_id, message, strlen(message));
        }
    } else{
        write(client_id, message, strlen(message));
    }

    close(client_id);
    return;
}

void handle_udp_connection(int udp_socket){
    struct sockaddr_in admin;

    /* Save messages from client */
	char admin_buffer[BUFFER_LEN];
    int recv_len;
    socklen_t udp_socket_len = sizeof(admin); 

    if((recv_len = recvfrom(udp_socket, admin_buffer, BUFFER_LEN-1, 0, (struct sockaddr *) &admin, (socklen_t *) &udp_socket_len)) == -1)
        error("Erro no recvfrom");

    // Para ignorar o restante conteúdo (anterior do buffer)
    admin_buffer[recv_len] = '\0';
    
    fflush(stdout);
    
    printf("UDP: %s\n", admin_buffer);
    
    char* message = (char*) malloc(BUFFER_LEN * sizeof(char));
    if(message == NULL)
        error("Erro a alocar memória!\n");

    /* Struct with info from user */
    struct_user user = verify_login_command(admin_buffer, message);
    if(strcmp(user.username, "a") != 0){
        if(verify_login(&user, "UDP")){
            strcpy(message, "OK [ADMIN]\n");
            sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *) &admin, udp_socket_len);
            
            while(1){
                recv_len = recvfrom(udp_socket, admin_buffer, BUFFER_LEN, 0, (struct sockaddr *) &admin, &udp_socket_len);
                if(recv_len == -1)
                    error("Erro no recvfrom");

                admin_buffer[recv_len] = '\0';
                remove_line_break(admin_buffer);

                printf("UDP: %s\n", admin_buffer);
                
                char** args = verify_admin_command(admin_buffer, message);
                if(args == NULL){
                    sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *) &admin, udp_socket_len);
                } else if(strcmp(args[0], "ERROR") == 0){
                    sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *) &admin, udp_socket_len);
                } else {
                    if(strcmp(message, "DEL\n") == 0){
                        strcpy(message,"From server: DEL\n");
                        sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *) &admin, udp_socket_len);
                        //POSTERIORMENTE CHAMA-SE A FUNÇÃO RESPONSÁVEL
                    } else if(strcmp(message,"ADD_USER\n") == 0){
                        strcpy(message, "From server: ADD_USER\n");
                        sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *) &admin, udp_socket_len);
                        printf("%s\n", add_user(args));
                        //POSTERIORMENTE CHAMA-SE A FUNÇÃO RESPONSÁVEL
                    } else if(strcmp(message, "LIST\n") == 0){
                        strcpy(message,"From server: LIST\n");
                        sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *) &admin, udp_socket_len);
                        //POSTERIORMENTE CHAMA-SE A FUNÇÃO RESPONSÁVEL
                    } if(strcmp(message, "QUIT_SERVER\n") == 0){
                        sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *) &admin, udp_socket_len);
                        queue_message msg ={
                            .priority = 1,
                            .msg = 1
                        };

                        if(msgsnd(mq_id, &msg, sizeof(msg) - sizeof(long), 0) == -1)
                            error("Erro a enviar mensagem para a message queue");

                        printf("[SERVER] Server closing in 60 seconds!\n");
                        for(int i = 1; i <= 2; i++){
                            sleep(20);
                            printf("[SERVER] Server closing in %d seconds!\n", 60 - i*20);
                        }
                        sleep(10);
                        printf("[SERVER] Server closing in 10 seconds!\n");
                        for(int i = 1; i < 10; i++){
                            sleep(1);
                            printf("[SERVER] Server closing in %d seconds!\n", 10 - i);
                        }
                        sleep(5);
                        kill(getppid(), SIGINT);
                        exit(0);
                    }
                }
            }
        } else{
            strcpy(message, "REJECTED");
            sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *) &admin, udp_socket_len);
        }
    } else{
        sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *) &admin, udp_socket_len);
    }
    close(udp_socket);
    return;
}

struct_user verify_login_command(char* input, char* message){
    struct_user user_null = {
        .username = "a",
        .password = "a",
        .type = "a"
    };

    struct_user user;
    
    char command[10];
    char* token = strtok(input, " ");

    if(strcmp(token, "LOGIN") != 0)
        return user_null;

    strcpy(command, token); // command -> "LOGIN"

    token = strtok(NULL, " ");
    if(token == NULL){
        strcpy(message, "LOGIN <username> <password>");
        return user_null;
    }
    strcpy(user.username, token);
    
    token = strtok(NULL, " ");
    if(token == NULL){
        strcpy(message, "LOGIN <username> <password>");
        return user_null;
    }
    strcpy(user.password, token);
    remove_line_break(user.password);

    return user;
}

void sigint_handler(){
    for(int i = 0; i < MAX_TCP_CLIENTS; i++){
        kill(tcp_pids[i], SIGINT);
    }
    kill(udp_pid, SIGINT);

    // Try to delete the key if she exists
    mq_id = msgget(key, 0); // Get message queue id.
    if(mq_id != -1){
        if(msgctl(mq_id, IPC_RMID, NULL) == -1){
            error("[%s] LOG - msgctl: ");
        }
        printf("[%s] LOG - Message queue deleted\n", get_time());
    }
    exit(0);
}