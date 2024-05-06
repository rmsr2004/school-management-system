#include "../functions/functions.h"
#include "../admin/admin.h"
#include "../client/client.h"
#include "../shared_memory/shm_lib.h"
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
#include <sys/shm.h>

#define PORTO_TURMAS        argv[1]             // Port to receive TCP packets.
#define PORTO_CONFIG        argv[2]             // Port to receive UDP packets.
#define CONFIG_FILE         argv[3]             // Config file path.
#define MAX_TCP_CLIENTS     10                  // Maximum number of TCP clients.

struct sockaddr_in server_udp;                  // UDP socket address.
struct sockaddr_in server_tcp;                  // TCP socket address.
struct sockaddr_in client_tcp;                  // TCP client address.
int client_addr_size = sizeof(client_tcp);      // TCP client address size.
int tcp_socket;                                 // TCP socket.
int udp_socket;                                 // UDP socket.

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
* Checks if login comstructmand was used correctly.
* @param input Login command received from user.
* @return If login is correct returns the user received,
*         otherwise returns null.
*/
struct_user verify_login_command(char* input, char* message);
/*
* Checks if user logged out.
*/
int handle_logout(char* buffer);
/*
* Creates a message queue.
*/
void creates_message_queue();
/*
* Handles signals.
*/
void signal_handler(int signum);
/*
* Function to check if there are finished processes.
*/
void check_finished_processes();

// Array to save all processes id that are responsible to handle TCP connection with MAX_TCP_CLIENTS length.
pid_t tcp_pids[MAX_TCP_CLIENTS];
// Amount of current TCP connections.
int tcp_pids_index = 0;
// UDP connection pid.
pid_t udp_pid;

/************************************************************************************
* Server receives UDP and TCP packets from clients.                                 *
* Execution: ./class_server {PORTO_TURMAS} {PORTO_CONFIG} {ficheiro configuração}   *        
*************************************************************************************/

int main(int argc, char *argv[]){
    signal(SIGINT, signal_handler); // Handle signals.
    signal(SIGTERM, signal_handler); // Handle signals.

    /* 
    *   Check if all arguments have been given 
    */
    if(argc != 4){
        error("./%s <PORTO_TURMAS> <PORTO_CONFIG> <ficheiro configuração>\n", argv[0]);
    }

    /*
    * Definitions to well functioning of the server
    */

    define_config_file(CONFIG_FILE);    // Define the config file path.
    creates_message_queue();            // Creates a message queue to communicate with children processes.  

    shmid = create_shared_memory();     // Create shared memory
    
    /* Initialize shared memory */
    void* shared_var = attach_shared_memory(shmid);

    classes_list* classes = (classes_list*) shared_var;     // Classes list.
    classes->current_size = 0;

    users_list* users = (users_list*)((char*) shared_var + sizeof(classes_list));    // Users list.
    users->current_size = 0;

    detach_shared_memory(shared_var); 
    printf("[%s] LOG - Shared memory created!\n", get_time());

    /*
    * Connections definitions.
    */
    
    // Preenchimento da socket address structure - udp
	server_udp.sin_family = AF_INET;
	server_udp.sin_port = htons(atoi(PORTO_CONFIG));
	server_udp.sin_addr.s_addr = htonl(INADDR_ANY);

    // Preenchimento da socket address structure - tcp
    bzero((void *) &server_tcp, sizeof(server_tcp));
    server_tcp.sin_family = AF_INET;
    server_tcp.sin_port = htons(atoi(PORTO_TURMAS));
    server_tcp.sin_addr.s_addr = htonl(INADDR_ANY);

    /* Handle TCP */

    if((tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){ 
        error("[%s] LOG - Error socket(): ", get_time());
    }

    if(bind(tcp_socket, (struct sockaddr*) &server_tcp, sizeof(server_tcp)) < 0){
        error("[%s] LOG - Error bind(): ", get_time());
    }

    if(listen(tcp_socket, MAX_TCP_CLIENTS) < 0){
        error("[%s] LOG - Error listen(): ", get_time());
    }
    
    /* Handle UDP */

	if((udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
        error("[%s] LOG - Error socket(): ", get_time());
    }

	if(bind(udp_socket, (struct sockaddr*) &server_udp, sizeof(server_udp)) == -1){
        error("[%s] LOG - Error bind(): ", get_time());
    }

    /* 
    *  Creates a single process to every time that a TCP connection is received, the process
    *  creates a new process to handle the connection.
    */
    pid_t tcp_pid;
    if((tcp_pid = fork()) == 0){
        while(tcp_pids_index < MAX_TCP_CLIENTS){
            //check_finished_processes();
            
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
            }
            close(client_id);
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
            // Register user on users list.
            /*
            void* shared_var = attach_shared_memory(shmid);
            users_list* users = (users_list*)((char*) shared_var + sizeof(classes_list));
            strcpy(users->users[users->current_size].username, user.username);
            strcpy(users->users[users->current_size].password, user.password);
            strcpy(users->users[users->current_size].type, user.type);
            users->current_size++;
            detach_shared_memory(shared_var);
            */

            printf("[%s] LOG - User '%s' logged in!\n", get_time(), user.username);

            sprintf(message, "OK [%s]", user.type);
            write(client_id, message, strlen(message));
            
            while(1){
                nread = read(client_id, client_buffer, BUFFER_LEN-1);
                client_buffer[nread] = '\0';
                
                printf("[%s] LOG - Received message from TCP: %s\n", get_time(), client_buffer);
                
                /*
                if(handle_logout(client_buffer)){
                    void* shared_var = attach_shared_memory(shmid);
                    users_list* users = (users_list*)((char*) shared_var + sizeof(classes_list));
                    int index = find_user(users, user.username);
                    if(index != -1){
                        for(int i = index; i < users->current_size - 1; i++){
                            strcpy(users->users[i].username, users->users[i + 1].username);
                            strcpy(users->users[i].password, users->users[i + 1].password);
                            strcpy(users->users[i].type, users->users[i + 1].type);
                        }
                        users->current_size--;
                    }
                    return;
                }
                */

                char** args = client_verify_command(client_buffer, user, message);
                if(args == NULL){              
                    write(client_id, message, strlen(message));
                } else if(strcmp(args[0], "ERROR") == 0){
                    write(client_id, message, strlen(message));
                } else{   
                    if(strcmp(message, "LIST_CLASSES") == 0){
                        strcpy(message, list_classes());
                        write(client_id, message, strlen(message));
                    } else if(strcmp(message, "LIST_SUBSCRIBED") == 0){
                        char* list = list_subscribed(user);
                        strcpy(message, list);
                        free(list);
                        write(client_id, message, strlen(message));
                    } else if(strcmp(message, "SUBSCRIBE_CLASS") == 0){
                        strcpy(message, subscribe_class(user, args[0]));
                        write(client_id, message, strlen(message));
                    } else if(strcmp(message, "CREATE_CLASS") == 0){
                        strcpy(message, "OK ");
                        strcat(message, create_new_class(user, args[0], atoi(args[1])));                        
                        write(client_id, message, strlen(message));
                    } else if(strcmp(message, "SEND") == 0){
                        if(send_message(args[0], args[1])){
                            strcpy(message, "OK");
                        } else{
                            strcpy(message, "ERROR");
                        }
                        write(client_id, message, strlen(message));
                    } else if(strcmp(message, "HELP") == 0){
                        strcpy(message, help_client(user));
                        write(client_id, message, strlen(message));
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
    free(message);
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
    
    printf("[%s] LOG - Received message from UDP: %s\n", get_time(), admin_buffer);
    
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

                printf("[%s] LOG - Received message from UDP: %s\n", get_time(), admin_buffer);
                
                char** args = verify_admin_command(admin_buffer, message);
                if(args == NULL){
                    sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *) &admin, udp_socket_len);
                } else if(strcmp(args[0], "ERROR") == 0){
                    sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *) &admin, udp_socket_len);
                } else {
                    if(strcmp(message, "DEL\n") == 0){
                        if(remove_user(args)){
                            strcpy(message, "OK\n");
                            printf("[%s] - LOG - User removed: %s\n", get_time(), args[0]);
                        } else{
                            strcpy(message, "ERROR\n");
                        }
                        sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *) &admin, udp_socket_len);
                    } else if(strcmp(message,"ADD_USER\n") == 0){
                        if(add_user(args)){
                            strcpy(message, "OK\n");
                            printf("[%s] - LOG - User added: %s\n", get_time(), args[0]);
                        } else{
                            strcpy(message, "ERROR\n");
                        }
                        sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *) &admin, udp_socket_len);
                    } else if(strcmp(message, "LIST\n") == 0){
                        char* result = list_users();
                        strcpy(message, result);
                        free(result);
                        sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *) &admin, udp_socket_len);
                    } else if(strcmp(message, "QUIT_SERVER\n") == 0){
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
                    } else if(strcmp(message, "HELP\n") == 0){
                        strcpy(message, help_admin(user));
                        sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *) &admin, udp_socket_len);
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
    free(message);
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

    /*
    * Verify if user is logged in.
    */
    void* shared_var = attach_shared_memory(shmid);
    users_list* users = (users_list*)((char*) shared_var + sizeof(classes_list));
    int index = find_user(users, user.username);
    if(index != -1){
        detach_shared_memory(shared_var);
        return user_null;
    }
    detach_shared_memory(shared_var);

    return user;
}

int handle_logout(char* buffer){
    char copy[strlen(buffer) + 1];
    strcpy(copy, buffer);

    char* token = strtok(copy, " ");
    token = strtok(NULL, " ");
    if(strcmp(token, "exiting...") == 0){
        return 1;
    }
    return 0;
}

void creates_message_queue(){
    /* Creation of message queue to communicate with children processes */
    key_msg = ftok(MSQ_QUEUE_PATH, 'A');
    if(key_msg == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    // Try to delete the key if she exists
    mq_id = msgget(key_msg, 0); // Get message queue id.
    if(mq_id != -1){
        if(msgctl(mq_id, IPC_RMID, NULL) == -1){
            error("[%s] LOG - msgctl: ");
        }
        printf("[%s] LOG - Message queue deleted\n", get_time());
    }
    
    mq_id = msgget(key_msg, IPC_CREAT | 0700);
    if(mq_id == -1){
        error("msgget");
    }

    printf("[%s] LOG - Message queue created\n", get_time());
}

void signal_handler(int signum){
    if(signum == SIGINT){
        // Try to delete the key if she exists
        mq_id = msgget(key_msg, 0); // Get message queue id.
        if(mq_id != -1){
            if(msgctl(mq_id, IPC_RMID, NULL) == -1){
                error("[%s] LOG - msgctl: ");
            }
            printf("[%s] LOG - Message queue deleted\n", get_time());
        }

        // Remove Shared Memory.
        remove_shared_memory(shmid);
        printf("[%s] LOG - Shared memory removed!\n", get_time());

        for(int i = 0; i < MAX_TCP_CLIENTS; i++){
            kill(tcp_pids[i], SIGKILL);
        }
        kill(udp_pid, SIGKILL);

        exit(0);
    }
}

void check_finished_processes(){
    for(int i = 0; i < tcp_pids_index; i++){
        pid_t result = waitpid(tcp_pids[i], NULL, WNOHANG);

        if(result > 0){
            // Process finished. Remove from array.
            for(int j = i; j < tcp_pids_index - 1; j++){
                tcp_pids[j] = tcp_pids[j + 1];
            }
            tcp_pids_index--;
            i--;
        }
    }
}