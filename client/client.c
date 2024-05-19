// João Afonso dos Santos Simões - 2022236316
// Rodrigo Miguel Santos Rodrigues - 2022233032
#include "client.h"
#include "../functions/functions.h"
#include "../class/class.h"
#include "../shared_memory/shm_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>
#include <sys/shm.h>

/*-----------------------------------------------------------------------------------------------------*
*                                       GLOBAL VARIABLES                                               *
*------------------------------------------------------------------------------------------------------*/
char multicast_ips_used[CLASSES_SIZE][16];          // CLASSES_SIZE multicast ips with 16 characters
int multicast_ips_used_size = 0;                    // number of multicast ip's used
int last_sv_port_used = 5000;                       // last port used for the server
int classes_created = 0;                            // number of classes created
/*-----------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------------*
*                                   FUNCTIONS THAT CLIENT (students) HAVE                              *
*------------------------------------------------------------------------------------------------------*/

char** client_verify_command(char* input, struct_user user, char* message){
    remove_line_break(input);

    char command[ARGS_LEN];

    char* token = strtok(input, " ");
    if(token == NULL){
        strcpy(message, "COMMAND NOT VALID\n");
        return NULL;
    }
    strcpy(command, token);

    if(strcmp(command, "LIST_CLASSES") == 0 && strcmp(user.type, "aluno") == 0){
        // LIST_CLASSES
        char** args = create_args_array(1);
        token = strtok(NULL, " ");
        if(token == NULL){
            strcpy(message, "LIST_CLASSES");
            strcpy(args[0], "LIST_CLASSES");
            return args;
        }
        // if it has arguments, it's wrong
        strcpy(message, "LIST_CLASSES não tem argumentos");
        strcpy(args[0], "ERROR");
        return args;
    } else if(strcmp(command, "LIST_SUBSCRIBED") == 0 && strcmp(user.type, "aluno") == 0){
        // LIST_SUBSCRIBED
        char** args = create_args_array(1);
        token = strtok(NULL, " ");
        if(token == NULL){
            strcpy(message, "LIST_SUBSCRIBED");
            strcpy(args[0], "LIST_SUBSCRIBED");
            return args;
        }
        // if it has arguments, it's wrong
        strcpy(message, "LIST_SUBSCRIBED não tem argumentos");
        strcpy(args[0], "ERROR");
        return args;
    } else if(strcmp(command, "SUBSCRIBE_CLASS") == 0 && strcmp(user.type, "aluno") == 0){
        // SUBSCRIBE_CLASS <name>
        // check if it has 1 argument
        char** args = create_args_array(1);
        token = strtok(NULL, " ");
        if(token == NULL){
            strcpy(message, "ERROR -> SUBSCRIBE_CLASS <name>");
            strcpy(args[0], "ERROR");
            return args;
        }
        strcpy(args[0],token);
        // check if it has more arguments
        token = strtok(NULL, " ");
        if(token == NULL){
            // if it doesn't, success
            strcpy(message, "SUBSCRIBE_CLASS");
            return args;
        }
        strcpy(message, "ERROR -> SUBSCRIBE_CLASS <name>");
        strcpy(args[0],"ERROR");
        return args;
    } else if(strcmp(command, "CREATE_CLASS") == 0 && strcmp(user.type, "professor") == 0){
        // CREATE_CLASS {name} {size}
        // check if it has 2 arguments
        char** args = create_args_array(2);
        token = strtok(NULL, " ");
        if(token == NULL){
            strcpy(message, "ERROR -> CREATE_CLASS {name} {size}");
            strcpy(args[0],"ERROR");
            return args;
        }
        strcpy(args[0], token);
        token = strtok(NULL, " ");
        if(token == NULL){
            strcpy(message, "ERROR -> CREATE_CLASS {name} {size}");
            strcpy(args[0],"ERROR");
            return args;
        }
        strcpy(args[1], token);
        // check if it has more arguments
        token = strtok(NULL, " ");
        if(token == NULL){
            // if it doesn't, success
            strcpy(message, "CREATE_CLASS");
            return args;
        }
        strcpy(message, "ERROR -> CREATE_CLASS {name} {size}");
        strcpy(args[0],"ERROR");
        return args;
    } else if(strcmp(command, "SEND") == 0 && strcmp(user.type, "professor") == 0){
        // SEND {name} {text that server will send to subscribers}
        // check if it has 2 arguments
        char** args = create_args_array(2);
        token = strtok(NULL, " ");
        if(token == NULL){
            strcpy(message, "ERROR -> SEND {name} {text that server will send to subscribers}");
            strcpy(args[0], "ERROR");
            return args;
        }
        strcpy(args[0], token);
        token = strtok(NULL, "\n");
        if(token == NULL){
            strcpy(message, "ERROR -> SEND {name} {text that server will send to subscribers}");
            strcpy(args[0], "ERROR");
            return args;
        }
        strcpy(message, "SEND");
        strcpy(args[1], token);
        return args;
    } else if(strcmp(command, "HELP") == 0){
        // HELP
        char** args = create_args_array(1);
        token = strtok(NULL, " ");
        if(token == NULL){
            strcpy(message, "HELP");
            strcpy(args[0], "HELP");
            return args;
        }
        // if it has arguments, it's wrong
        strcpy(message, "HELP não tem argumentos");
        strcpy(args[0], "ERROR");
        return args;
    } else{
        strcpy(message, "COMMAND NOT VALID");
        return NULL;
    }
}

char* help_client(struct_user user){
    if(strcmp(user.type, "aluno") == 0){
        return "Available commands:\nLIST_CLASSES\nLIST_SUBSCRIBED\nSUBSCRIBE_CLASS <name>";
    } else if(strcmp(user.type, "professor") == 0){
        return "CREATE_CLASS <name> <size>\nSEND <name> <text that server will send to subscribers>";
    }
    return NULL;
}

char* create_new_class(struct_user professor, char* name, int size){
    if(classes_created == CLASSES_SIZE){
        return "CLASSES LIST IS FULL";
    }

    if(!verify_string(name)){
        return "CLASS NAME NOT VALID";
    }

    /*
    *   Create TCP multicast group
    */

    struct sockaddr_in addr;    // multicast address

    char *multicast_ip = generate_multicast_ip();
    
    // set multicast address
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(multicast_ip);
    addr.sin_port = htons(last_sv_port_used);
    last_sv_port_used++;

    /*
    *   Create the class 
    */

    struct class new_class;
    strcpy(new_class.name, name);
    new_class.size = size;
    new_class.professor = professor;
    new_class.current_size = 0;
    new_class.addr = addr;

    void* shared_var = attach_shared_memory(shmid);
    classes_list* classes = (classes_list*) shared_var;

    if(!add_class(classes, new_class)){
        detach_shared_memory(shared_var);
        return "CLASS ALREADY EXISTS OR CLASSES LIST IS FULL";
    }
    detach_shared_memory(shared_var);

    classes_created++;
    printf("[%s] LOG - Class '%s' created by %s.\n", get_time(), name, professor.username);

    char *return_message = calloc(25, sizeof(char));
    sprintf(return_message, "OK %s", multicast_ip);

    return return_message;
}

char* subscribe_class(struct_user user, char* class_name){
    remove_line_break(class_name);

    void* shared_var = attach_shared_memory(shmid);
    classes_list* classes = (classes_list*) shared_var;
    
    int class_index = find_class(classes, class_name);
    if(class_index != -1){
        struct class* class = &classes->classes[class_index];
        if(add_student(class, user)){
            char* return_message = calloc(50, sizeof(char));
            if(return_message == NULL){
                detach_shared_memory(shared_var);
                error("[%s] LOG - subscribe_class() -> calloc(): ", get_time());
            }

            sprintf(return_message, "ACCEPTED %s", inet_ntoa(class->addr.sin_addr));

            printf("[%s] LOG - User '%s' subscribed to class '%s'.\n", get_time(), user.username, class_name);
                    
            detach_shared_memory(shared_var);
            return return_message;
        }
    }
    detach_shared_memory(shared_var);
    return "REJECTED";
}

int send_message(char* class_name, char* message){
    remove_line_break(message);

    void* shared_var = attach_shared_memory(shmid);
    classes_list* classes = (classes_list*) shared_var;

    int index = find_class(classes, class_name);
    if(index != -1){
        struct sockaddr_in addr = classes->classes[index].addr;

        /*
        *   Socket creation
        */

        int sock;
        if((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
            detach_shared_memory(shared_var);
            error("[%s] LOG - send_message -> socket(): ", get_time());
        }

        int ttl = 64;
        if(setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0){
            error("setsockopt(): ");
        }

        char* msg_to_send = calloc(((2*ARGS_LEN) + 50), sizeof(char));
        if(msg_to_send == NULL){
            detach_shared_memory(shared_var);
            error("[%s] LOG - send_message -> calloc(): ", get_time());
        }

        sprintf(msg_to_send, "[%s | %s]: %s", classes->classes[index].professor.username, class_name, message);

        // Send the message
        if(sendto(sock, msg_to_send, BUFFER_LEN, 0, (struct sockaddr*)&addr, sizeof(addr)) == -1){
            detach_shared_memory(shared_var);
            error("[%s] LOG - send_message -> sendto(): ", get_time());
        }

        detach_shared_memory(shared_var);

        printf("[%s] LOG - Message sent to class '%s'. Message Content: '%s'\n", get_time(), class_name, msg_to_send);
        close(sock);
        return 1;
    }
    detach_shared_memory(shared_var);
    return 0;
}

char* list_classes(){
    void* shared_var = attach_shared_memory(shmid);
    classes_list* classes = (classes_list*) shared_var;

    if(classes->current_size == 0){
        detach_shared_memory(shared_var);
        return "NO CLASSES AVAILABLE";
    }

    char* message = calloc(ARGS_LEN * classes->current_size, sizeof(char));
    if(message == NULL){
        detach_shared_memory(shared_var);
        error("[%s] LOG - list_classes() -> calloc(): ", get_time());
    }

    for(int i = 0; i < classes->current_size; i++){
        char temp[ARGS_LEN+10];
        memset(temp, 0, ARGS_LEN+10);

        sprintf(temp, "{%s}, ", classes->classes[i].name);
        strcat(message, temp);
    }

    detach_shared_memory(shared_var);

    // Remove the last ", "
    if((int) strlen(message) > 0){
        message[strlen(message) - 2] = '\0';
    }

    remove_line_break(message);
    return message;
}

char* list_subscribed(struct_user user){
    void* shared_var = attach_shared_memory(shmid);
    classes_list* classes = (classes_list*) shared_var;

    int total_classes = 0;
    for(int i = 0; i < classes->current_size; i++){
        for(int j = 0; j < classes->classes[i].current_size; j++){
            if(strcmp(classes->classes[i].students[j].username, user.username) == 0){
                total_classes++;
            }
        }
    }

    if(total_classes == 0){
        detach_shared_memory(shared_var);
        return strdup("NO CLASSES SUBSCRIBED");
    }

    size_t message_size = (ARGS_LEN + 20) * total_classes;
    char* message = calloc(message_size, sizeof(char));
    if(message == NULL){
        detach_shared_memory(shared_var);
        error("[%s] LOG - list_subscribed() -> calloc(): ", get_time());
    }

    for(int i = 0; i < classes->current_size; i++){
        for(int j = 0; j < classes->classes[i].current_size; j++){
            if(strcmp(classes->classes[i].students[j].username, user.username) == 0){
                char temp[ARGS_LEN + 20];
                snprintf(temp, sizeof(temp), "{%s/%s}, ", classes->classes[i].name, inet_ntoa(classes->classes[i].addr.sin_addr));
                strcat(message, temp);
            }
        }
    }

    detach_shared_memory(shared_var);

    // Remover o último ", "
    if((int)strlen(message) > 0){
        message[strlen(message) - 2] = '\0';
    }

    remove_line_break(message);
    return message;
}

char* generate_multicast_ip(){ 
    srand(time(NULL)); 
    
    int octet1 = 224 + rand() % 16;
    int octet2 = rand() % 256;
    int octet3 = rand() % 256;
    int octet4 = rand() % 256;

    char* multicast_ip = calloc(30, sizeof(char));       
    sprintf(multicast_ip, "%d.%d.%d.%d", octet1, octet2, octet3, octet4);
    if(ip_used(multicast_ip)){
        free(multicast_ip);
        return generate_multicast_ip();
    }

    strcpy(multicast_ips_used[multicast_ips_used_size], multicast_ip);
    multicast_ips_used_size++;
    
    return multicast_ip;
}

int ip_used(char* ip){
    for(int i = 0; i < multicast_ips_used_size; i++){
        if(strcmp(multicast_ips_used[i], ip) == 0){
            return 1;
        }
    }
    return 0;
}

/*-----------------------------------------------------------------------------------------------------*
*                                       TCP_CLIENT.C FUNCTIONS                                         *
*------------------------------------------------------------------------------------------------------*/

void sig_handler(int signum){
    if(signum == SIGTERM){
        kill(getppid(), SIGTERM);
        exit(EXIT_SUCCESS);
    }

    if(signum == SIGINT){
        exit(0);
    }
}

void handle_sigint(int signum){
    if(signum == SIGINT){
        if(getpid() == father_pid){
            printf("\n");
            printf("Exiting...\n");
                
            /*
            *   Remove user from all the multicast groups
            */ 

            for(int i = 0; i < multicast_index; i++){
                struct ip_mreq mreq = multicast_mreq[i];
                if(setsockopt(multicast_sock[i], IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
                    perror("setsockopt");
                    exit(1);
                }
            }

            char* message = calloc(35, sizeof(char));
            sprintf(message, "TCP_CLIENT[%d] exiting...", getpid());
            write(fd, message, strlen(message));
            cleanup_resources();
            exit(-1);
        } else{
            exit(-1);
        }
    }
}

void read_multicast_messages(int index){
    signal(SIGINT, handle_sigint);

    char* msg_from_multicast = calloc(BUFFER_LEN, sizeof(char));
    if(msg_from_multicast == NULL){
        cleanup_resources();
        error("msg_from_multicast -> calloc(): ");
    }

    while(1){
        struct sockaddr_in sender_addr;
        socklen_t sender_len = sizeof(sender_addr);
        int nbytes;
        if((nbytes = recvfrom(multicast_sock[index], msg_from_multicast, BUFFER_LEN, 0, (struct sockaddr *)&sender_addr, &sender_len)) > 0){
            msg_from_multicast[nbytes] = '\0';
            printf("\nNew message received:\n%s\n", msg_from_multicast);
            fflush(stdout);
            printf(">> ");
            fflush(stdout);
        }
    }
    close(multicast_sock[index]);
}

void cleanup_resources(){
    if(fd != -1){
        close(fd);
    }
    
    if(buffer != NULL){
        free(buffer);
    }
    if(input != NULL){
        free(input);
    }

    for(int i = 0; i < multicast_index; i++){
        if(multicast_pid[i] > 0){
            kill(multicast_pid[i], SIGINT);
        }

        if(multicast_sock[i] != -1){
            close(multicast_sock[i]);
        }
    }

    if(pid > 0){
        kill(pid, SIGKILL);
    }
}

// client.c