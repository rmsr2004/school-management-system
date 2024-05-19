// João Afonso dos Santos Simões - 2022236316
// Rodrigo Miguel Santos Rodrigues - 2022233032
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

#define PORTO_TURMAS    argv[1]             // Port to receive TCP packets.
#define PORTO_CONFIG    argv[2]             // Port to receive UDP packets.
#define CONFIG_FILE     argv[3]             // Config file path.

/*------------------------------------------------------------------------------------------------------*
*                                           FUNCTIONS                                                   *
*-------------------------------------------------------------------------------------------------------*/

/*
*   Handles a TCP connection.
*   @param client_id New connection file descriptor.
*/
void handle_tcp_connection(int client_id);
/*
*   Handles a UDP connection.
*   @param udp_socket UDP socket.
*/
void handle_udp_connection(int udp_socket);
/*
*   Checks if login command was used correctly.
*   @param input Login command received from user.
*   @return If login is correct returns the user received,
*           otherwise returns null.
*/
struct_user verify_login_command(char* input, char* message);
/*
*   Checks if user logged out.
*/
int handle_logout(char* buffer);
/*
*   Handles signals.
*/
void signal_handler(int signum);
/*
*   Function to check if there are finished processes.
*/
void check_finished_processes();
/*
*   Function to handle signals in TCP process.
*/
void tcp_sig_handler();


/*-----------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------------*
*                                           SERVER                                                      *
*********************************************************************************************************
*   The server will be responsible for the following functionalities:                                   *
*   - Manage classes:                                                                                   *                                        
*       - Informing customers of the various existing classes, as well as managing the customers        *
*         enrolled in each class.                                                                       *
*       - Ensure that the capacity of the classes is not exceeded. o Authenticate clients and manage    *
*         their permissions (which clients can create classes and publish content, or only receive      * 
*         content in the classes in which they are enrolled).                                           *
*       - Inform clients who enrol in a particular class which multicast address to use to receive      *
*         content sent to that class.                                                                   *
*   - User management:                                                                                  *
*       - Manage users identified by their login, password and permissions. This information should     *
*         be stored in a text file.                                                                     *
*       - Allow administrator access via the administration console.                                    *
*                                                                                                       *                          
*   Command Syntax:                                                                                     *
*       - class_server {PORTO_TURMAS} {PORTO_CONFIG} {ficheiro configuração}                            *
*********************************************************************************************************/
int main(int argc, char *argv[]){
    signal(SIGINT, signal_handler); // Handle signals.


    /* 
    *   Check if all arguments have been given 
    */
    if(argc != 4){
        error("./%s <PORTO_TURMAS> <PORTO_CONFIG> <ficheiro configuração>\n", argv[0]);
    }

    while(waitpid(-1, NULL, WNOHANG) > 0); // Clear zombie processes.

    /*
    *   Definitions to well functioning of the server
    */

    define_config_file(CONFIG_FILE);    // Define the config file path.

    shmid = create_shared_memory(); // Create shared memory
    
    /* 
    *   Initialize shared memory 
    */

    void* shared_var = attach_shared_memory(shmid);

    classes_list* classes = (classes_list*) shared_var; // Classes list.
    classes->current_size = 0;

    users_list* users = (users_list*)((char*) shared_var + sizeof(classes_list));   // Users list.
    users->current_size = 0;

    detach_shared_memory(shared_var); 

    /*
    *   Connections definitions.
    */
    
    // Filling in the socket address structure - udp
    memset(&server_udp, 0, sizeof(server_udp));
	server_udp.sin_family = AF_INET;
	server_udp.sin_port = htons(atoi(PORTO_CONFIG));
	server_udp.sin_addr.s_addr = htonl(INADDR_ANY);

    // Filling in the socket address structure - tcp
    bzero((void *) &server_tcp, sizeof(server_tcp));
    server_tcp.sin_family = AF_INET;
    server_tcp.sin_port = htons(atoi(PORTO_TURMAS));
    server_tcp.sin_addr.s_addr = htonl(INADDR_ANY);

    /* 
    *   Handle TCP 
    */

    if((tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){ 
        error("[%s] LOG - Error socket(): ", get_time());
    }

    if(bind(tcp_socket, (struct sockaddr*) &server_tcp, sizeof(server_tcp)) < 0){
        error("[%s] LOG - Error bind(): ", get_time());
    }

    if(listen(tcp_socket, MAX_USERS) < 0){
        error("[%s] LOG - Error listen(): ", get_time());
    }

    /* 
    *   Handle UDP. Create socket, set options and bind.
    */

	if((udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
        error("[%s] LOG - Error socket(): ", get_time());
    }

    int reuse = 1;
    if(setsockopt(udp_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0){
        error("[%s] LOG - Error setsockopt(): ", get_time());
    }

	if(bind(udp_socket, (struct sockaddr*) &server_udp, sizeof(server_udp)) == -1){
        error("[%s] LOG - Error bind(): ", get_time());
    }

    /* 
    *  Creates a single process to every time that a TCP connection is received the process
    *  creates a new process to handle the connection.
    */
    
    pid_t pid;
    if((pid = fork()) == 0){
        signal(SIGINT, tcp_sig_handler);    // Handle signals.
        /* CHILD PROCESS */
        while(tcp_pids_index < MAX_USERS){
            check_finished_processes(); // Check if there are finished processes.

            pid_t pid;

            client_id = accept(tcp_socket, (struct sockaddr *) &client_tcp, (socklen_t *) &client_addr_size);
            if(client_id > 0){
                if((pid = fork()) == 0){
                    handle_tcp_connection(client_id);
                    close(tcp_socket);
                    exit(0);
                }
                printf("[%s] LOG - New TCP connection received. Process %d created!\n", get_time(), pid);
                
                // Save the process id 
                tcp_pids[tcp_pids_index] = pid;
                tcp_pids_index++;
            }
            close(client_id);
        }
        exit(0);
        /* EXIT CHILD PROCESS */
    }
    tcp_pid = pid;
    printf("[%s] LOG - Process %d created to handle TCP connections!\n", get_time(), tcp_pid);

    /*
    *   Creates a single process to handle UDP connections.
    */
    while(1){
        if((udp_pid = fork()) == 0){
            handle_udp_connection(udp_socket);
            close(udp_socket);
            exit(0);
        }
        printf("[%s] LOG - Process %d created to handle UDP connections!\n", get_time(), udp_pid);
        waitpid(udp_pid, NULL, 0); // Waiting for UDP process.
    }
    close(udp_socket);

    // Waiting for TCP processes to finish
    waitpid(tcp_pid, NULL, 0); // Waiting for TCP process.
    waitpid(udp_pid, NULL, 0); // Waiting for UDP process.

    return 0;
}

void handle_tcp_connection(int client_id){
    signal(SIGINT, SIG_DFL);    // Handle signals.

    char client_buffer[BUFFER_LEN]; // Buffer to save messages from client.
    int nread;  // Number of bytes read from client.

    /*
    *   Verify login
    */

    nread = read(client_id, client_buffer, BUFFER_LEN-1);
    client_buffer[nread] = '\0';

    printf("[%s] LOG - Received message from TCP: %s\n", get_time(), client_buffer);

    char* message = (char*) malloc(BUFFER_LEN * sizeof(char));
    if(message == NULL){
        error("[%s] LOG - handle_tcp_connection -> malloc(): ", get_time());
    }

    struct_user user = verify_login_command(client_buffer, message); // Struct with info from user received.
                                                                     // If user username is "a" then the login
                                                                     // failed.

    if(strcmp(user.username, "a") != 0){
        /*
        *   LOGIN COMMAND WAS USED CORRECTLY
        */

        // Verify login -> search for user on config file.
        if(verify_login(&user, "TCP")){
            /* 
            *   User exists and is logged in.
            *   Register user on users list.
            */

            void* shared_var = attach_shared_memory(shmid);
            users_list* users = (users_list*)((char*) shared_var + sizeof(classes_list));
            
            strcpy(users->users[users->current_size].username, user.username);
            strcpy(users->users[users->current_size].password, user.password);
            strcpy(users->users[users->current_size].type, user.type);
            users->current_size++;
            
            detach_shared_memory(shared_var);
            
            printf("[%s] LOG - User '%s' logged in!\n", get_time(), user.username);

            // Send message to client.
            sprintf(message, "OK [%s]", user.type);
            write(client_id, message, strlen(message));
            
            /*
            *  Handle client messages.
            */
            while(1){
                nread = read(client_id, client_buffer, BUFFER_LEN-1);
                if(nread < 0){
                    error("[%s] LOG - handle_tcp_connection -> Error read(): ", get_time());
                    kill(getppid(), SIGINT);
                }
                client_buffer[nread] = '\0';
                
                printf("[%s] LOG - Received message from TCP: %s\n", get_time(), client_buffer);

                // Check if user logged out. If so, remove user from users list and subscribed classes.
                if(handle_logout(client_buffer)){
                    printf("[%s] LOG - User '%s' logged out!\n", get_time(), user.username);

                    void* shared_var = attach_shared_memory(shmid);

                    /*
                    *   Remove user from users list.
                    */

                    users_list* users = (users_list*)((char*) shared_var + sizeof(classes_list));
                    if(remove_user(users, user.username)){
                        printf("[%s] LOG - User '%s' removed from users list!\n", get_time(), user.username);
                    } else{
                        printf("[%s] LOG - User '%s' not found in users list!\n", get_time(), user.username);
                    }

                    /*
                    *  Remove user from subscribed classes.
                    */

                    classes_list* classes = (classes_list*) shared_var;

                    for(int i = 0; i < classes->current_size; i++){
                        struct class* current_class = &classes->classes[i];

                        for(int j = 0; j < current_class->current_size; j++){
                            if(strcmp(current_class->students[j].username, user.username) == 0){
                                remove_student_from_class(classes, current_class->name, user.username);
                                j--;
                            }
                        }
                    }

                    detach_shared_memory(shared_var);
                    return;
                }
                
                /*
                *   Verify command. 
                *   If does not exist, args = NULL. Message = "COMMAND NOT VALID"
                *   If is a valid one, args = command arguments. Message = command.
                *   If is not valid (missing arguments, ...), args[0] = "ERROR". Message = "ERROR ..."
                */

                char** args = client_verify_command(client_buffer, user, message);
                if(args == NULL){              
                    write(client_id, message, strlen(message));
                } else if(strcmp(args[0], "ERROR") == 0){
                    write(client_id, message, strlen(message));
                } else{   
                    /*
                    *   Command is valid. Execute command and send message to client.
                    */

                    if(strcmp(message, "LIST_CLASSES") == 0){
                        strcpy(message, list_classes());
                        write(client_id, message, strlen(message));
                    } else if(strcmp(message, "LIST_SUBSCRIBED") == 0){
                        strcpy(message, list_subscribed(user));
                        write(client_id, message, strlen(message));
                    } else if(strcmp(message, "SUBSCRIBE_CLASS") == 0){
                        strcpy(message, subscribe_class(user, args[0]));
                        write(client_id, message, strlen(message));
                    } else if(strcmp(message, "CREATE_CLASS") == 0){
                        if(!is_number(args[1])){
                            strcpy(message, "INVALID CLASS SIZE");
                        } else{
                            strcpy(message, create_new_class(user, args[0], atoi(args[1])));
                        }                   
                        write(client_id, message, strlen(message));
                    } else if(strcmp(message, "SEND") == 0){
                        if(send_message(args[0], args[1])){
                            strcpy(message, "OK");
                        } else{
                            strcpy(message, "CLASS NOT FOUND");
                        }
                        write(client_id, message, strlen(message));
                    } else if(strcmp(message, "HELP") == 0){
                        strcpy(message, help_client(user));
                        write(client_id, message, strlen(message));
                    }
                }
            }
        } else{
            /*
            *   User doesn't exist or password is incorrect.
            */

            strcpy(message, "REJECTED");
            write(client_id, message, strlen(message));
        } 
    } else{
        /*
        *   LOGIN COMMAND WAS NOT USED CORRECTLY
        */

        write(client_id, message, strlen(message));
    }
    free(message);
    close(client_id);
    return;
}

void handle_udp_connection(int udp_socket){
    signal(SIGINT, SIG_DFL);    // Handle signals.

    struct sockaddr_in admin;   // Admin address.
    socklen_t udp_socket_len = sizeof(admin);   // Admin address size.

	char admin_buffer[BUFFER_LEN];  // Save messages from client 
    int recv_len;   // Number of bytes received from client.

    char* message = (char*) malloc(BUFFER_LEN * sizeof(char));
    if(message == NULL){
        error("[%s] LOG - handle_udp_connection -> error malloc(): ");
    }

    /*
    *	Avoid the problem of the first message not being sent on GNS3
    */
    
    if((recv_len = recvfrom(udp_socket, admin_buffer, BUFFER_LEN-1, 0, (struct sockaddr *) &admin, (socklen_t *) &udp_socket_len)) == -1){
        error("[%s] LOG - handle_udp_connection -> Error recvfrom(): ", get_time());
    }
    if((recv_len = recvfrom(udp_socket, admin_buffer, BUFFER_LEN-1, 0, (struct sockaddr *) &admin, (socklen_t *) &udp_socket_len)) == -1){
        error("[%s] LOG - handle_udp_connection -> Error recvfrom(): ", get_time());
    }
    if((recv_len = recvfrom(udp_socket, admin_buffer, BUFFER_LEN-1, 0, (struct sockaddr *) &admin, (socklen_t *) &udp_socket_len)) == -1){
        error("[%s] LOG - handle_udp_connection -> Error recvfrom(): ", get_time());
    }

 	printf("[%s] LOG - Connection established with admin UDP\n", get_time());

    /*
    *   Verify login.
    */

    if((recv_len = recvfrom(udp_socket, admin_buffer, BUFFER_LEN-1, 0, (struct sockaddr *) &admin, (socklen_t *) &udp_socket_len)) == -1){
        error("[%s] LOG - handle_udp_connection -> Error recvfrom(): ", get_time());
    }
    admin_buffer[recv_len] = '\0';
    
    printf("[%s] LOG - Received message from UDP: %s\n", get_time(), admin_buffer);
    

    struct_user user = verify_login_command(admin_buffer, message); // Struct with info from user received.
                                                                    // If user username is "a" then the login
                                                                    // failed.
    if(strcmp(user.username, "a") != 0){
        /*
        *   LOGIN COMMAND WAS USED CORRECTLY
        */

        // Verify login -> search for user on config file.
        if(verify_login(&user, "UDP")){
            /* 
            *   User exists and is logged in.
            *   Register user on users list.
            */

            void* shared_var = attach_shared_memory(shmid);
            users_list* users = (users_list*)((char*) shared_var + sizeof(classes_list));

            if(!add_user(users, user)){
                detach_shared_memory(shared_var);
                printf("[%s] LOG - Maximum of users connected\n", get_time());
                strcpy(message, "REJECTED");
                close(udp_socket);

                return;
            }
            detach_shared_memory(shared_var);
            

            printf("[%s] LOG - User '%s' logged in!\n", get_time(), user.username);

            // Send message to client.
            strcpy(message, "OK [ADMIN]\n");
            sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *) &admin, udp_socket_len);
            
            /*
            *  Handle admin messages.
            */
            while(1){
                recv_len = recvfrom(udp_socket, admin_buffer, BUFFER_LEN, 0, (struct sockaddr *) &admin, &udp_socket_len);
                if(recv_len == -1){
                    error("[%s] LOG - handle_udp_connection -> Error recvfrom(): ", get_time());
                }
                admin_buffer[recv_len] = '\0';
                remove_line_break(admin_buffer);

                printf("[%s] LOG - Received message from UDP: %s\n", get_time(), admin_buffer);

                // Check if user logged out. If so, remove user from users list.
                if(handle_logout(admin_buffer)){
                    printf("[%s] LOG - User '%s' logged out!\n", get_time(), user.username);

                    void* shared_var = attach_shared_memory(shmid);

                    /*
                    *   Remove user from users list.
                    */

                    users_list* users = (users_list*)((char*) shared_var + sizeof(classes_list));
                    if(remove_user(users, user.username)){
                        printf("[%s] LOG - User '%s' removed from users list!\n", get_time(), user.username);
                    } else{
                        printf("[%s] LOG - User '%s' not found in users list!\n", get_time(), user.username);
                    }
                    detach_shared_memory(shared_var);
                    exit(0);
                }

                /*
                *   Verify command. 
                *   If does not exist, args = NULL. Message = "COMMAND NOT VALID"
                *   If is a valid one, args = command arguments. Message = command.
                *   If is not valid (missing arguments, ...), args[0] = "ERROR". Message = "ERROR ..."
                */

                char** args = verify_admin_command(admin_buffer, message);
                if(args == NULL){
                    sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *) &admin, udp_socket_len);
                } else if(strcmp(args[0], "ERROR") == 0){
                    sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *) &admin, udp_socket_len);
                } else {
                    /*
                    *   Command is valid. Execute command and send message to client.
                    */

                    if(strcmp(message, "DEL\n") == 0){
                        if(delete_user(args)){
                            strcpy(message, "OK\n");
                            printf("[%s] - LOG - User removed: %s\n", get_time(), args[0]);
                        } else{
                            strcpy(message, "ERROR\n");
                        }
                        sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *) &admin, udp_socket_len);
                    } else if(strcmp(message,"ADD_USER\n") == 0){
                        if(register_user(args)){
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
            /*
            *   User doesn't exist or password is incorrect.
            */

            strcpy(message, "REJECTED\n");
            sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *) &admin, udp_socket_len);
        }
    } else{
        /*
        *   LOGIN COMMAND WAS NOT USED CORRECTLY
        */

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

    char* token = strtok(input, " ");
 
    if(strcmp(token, "LOGIN") != 0){
        strcpy(message, "LOGIN <username> <password>");
        return user_null;
    }
        
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

    // Check if there are more arguments.
    token = strtok(NULL, " ");
    if(token != NULL){
        strcpy(message, "LOGIN <username> <password>");
        return user_null;
    }
    return user;
}

int handle_logout(char* buffer){
    char copy[strlen(buffer) + 1];
    strcpy(copy, buffer);

    char* token = strtok(copy, " ");
    if(token == NULL){
        return 0;
    }

    token = strtok(NULL, " ");
    if(token == NULL){
        return 0;
    }

    if(strcmp(token, "exiting...") == 0){
        return 1;
    }
    return 0;
}


void signal_handler(int signum){
    if(signum == SIGINT){
        printf("\n");
        printf("[%s] LOG - Server closing!\n", get_time());


        // Remove Shared Memory.
        remove_shared_memory(shmid);
        printf("[%s] LOG - Shared memory removed!\n", get_time());

        kill(0, SIGKILL);
        
        kill(tcp_pid, SIGINT);
        kill(udp_pid, SIGKILL);

        waitpid(tcp_pid, NULL, 0);
        waitpid(udp_pid, NULL, 0);


        exit(0);
    }
}

void check_finished_processes(){
    int status;
    pid_t pid;

    for(int i = 0; i < tcp_pids_index; i++){
        pid = waitpid(tcp_pids[i], &status, WNOHANG);
        if(pid == tcp_pids[i]){
            printf("[%s] LOG - Process %d has finished.\n", get_time(), pid);

            for(int j = i; j < tcp_pids_index - 1; j++){
                tcp_pids[j] = tcp_pids[j + 1];
            }
            tcp_pids_index--;
            i--;
        } else if(pid == -1){
            error("[%d] LOG - check_finished_processes waitpid(): ", get_time());
        }
    }
}
void tcp_sig_handler(){
    for(int i = 0; i < tcp_pids_index; i++){
        printf("%d ", tcp_pids[i]);
    }
    printf("\n");

    close(client_id);

    for(int i = 0; i < tcp_pids_index; i++){
        kill(tcp_pids[i], SIGKILL);
    }

    // Wait for all child processes to terminate
    for (int i = 0; i < tcp_pids_index; ++i) {
        waitpid(tcp_pids[i], NULL, 0);
    }
    exit(0);
}

// server.c