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

char multicast_ips_used[20][16];
int multicast_ips_used_size = 0;
int last_sv_port_used = 5000;

char** client_verify_command(char* input, struct_user user, char* message){
    char command[20];
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
        // se tiver argumentos, dá errado
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
        // se tiver argumentos, dá errado
        strcpy(message, "LIST_SUBSCRIBED não tem argumentos");
        strcpy(args[0], "ERROR");
        return args;
    } else if(strcmp(command, "SUBSCRIBE_CLASS") == 0 && strcmp(user.type, "aluno") == 0){
        // SUBSCRIBE_CLASS <name>
        // verificar se tem o argumento
        char** args = create_args_array(1);
        token = strtok(NULL, " ");
        if(token == NULL){
            strcpy(message, "ERROR -> SUBSCRIBE_CLASS <name>");
            strcpy(args[0], "ERROR");
            return args;
        }
        strcpy(args[0],token);
        // verificar se tem argumentos a mais
        token = strtok(NULL, " ");
        if(token == NULL){
            // Não tem, então sucesso
            strcpy(message, "SUBSCRIBE_CLASS");
            return args;
        }
        strcpy(message, "ERROR -> SUBSCRIBE_CLASS <name>");
        strcpy(args[0],"ERROR");
        return args;
    } else if(strcmp(command, "CREATE_CLASS") == 0 && strcmp(user.type, "professor") == 0){
        // CREATE_CLASS {name} {size}
        // verificar se tem os 2 argumentos
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
        // verificar se tem argumentos a mais
        token = strtok(NULL, " ");
        if(token == NULL){
            // Não tem, então sucesso
            strcpy(message, "CREATE_CLASS");
            return args;
        }
        strcpy(message, "ERROR -> CREATE_CLASS {name} {size}");
        strcpy(args[0],"ERROR");
        return args;
    } else if(strcmp(command, "SEND") == 0 && strcmp(user.type, "professor") == 0){
        // SEND {name} {text that server will send to subscribers}
        // verificar se tem os 2 argumentos
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
        // se tiver argumentos, dá errado
        strcpy(message, "HELP não tem argumentos");
        strcpy(args[0], "ERROR");
        return args;
    } else{
        strcpy(message, "COMMAND NOT VALID");
        return NULL;
    }
}

char* help_client(struct_user user){
    char* message = (char*) malloc(84 * sizeof(char));
    if(message == NULL){
        error("help_client -> malloc(): ");
    }

    if(strcmp(user.type, "aluno") == 0){
        sprintf(message, "Available commands:\nLIST_CLASSES\nLIST_SUBSCRIBED\nSUBSCRIBE_CLASS <name>");
    } else if(strcmp(user.type, "professor") == 0){
        sprintf(message, "CREATE_CLASS <name> <size>\nSEND <name> <text that server will send to subscribers>");
    }
    return message;
}

char* create_new_class(struct_user professor, char* name, int size){
    // create TCP multicast group
    int sock;
    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        error("[%s] LOG - create_new_class() -> socket(): ", get_time());
    }

    struct sockaddr_in addr;

    char *multicast_ip = generate_multicast_ip();
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(multicast_ip);
    addr.sin_port = htons(last_sv_port_used + 1);
    last_sv_port_used++;

    int enable = 1;
    if(setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, &enable, sizeof(enable)) < 0){
        error("setsockopt(): ");
    }

    struct class new_class;
    strcpy(new_class.name, name);
    new_class.size = size;
    new_class.professor = professor;
    new_class.current_size = 0;
    new_class.addr = addr;

    void* shared_var = attach_shared_memory(shmid);
    classes_list* classes = (classes_list*) shared_var;

    classes->classes[classes->current_size] = new_class;
    classes->current_size += 1;

    detach_shared_memory(shared_var);

    printf("[%s] LOG - Class '%s' created by %s.\n", get_time(), name, professor.username);

    return multicast_ip;
}

char* subscribe_class(struct_user user, char* class_name){
    remove_line_break(class_name);

    void* shared_var = attach_shared_memory(shmid);
    classes_list* classes = (classes_list*) shared_var;
    
    int class_index = find_class(classes, class_name);
    if(class_index != -1){
        struct class* class = &classes->classes[class_index];
        if(add_student(class, user)){
            char* return_message = (char*) malloc(35 * sizeof(char));
            if(return_message == NULL){
                detach_shared_memory(shared_var);
                error("[%s] LOG - subscribe_class() -> malloc(): ", get_time());
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

    for(int i = 0; i < classes->current_size; i++){
        if(strcmp(classes->classes[i].name, class_name) == 0){
            struct sockaddr_in addr = classes->classes[i].addr;

            int sock;
            if((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
                detach_shared_memory(shared_var);
                error("[%s] LOG - send_message -> socket(): ", get_time());
            }

            char* msg_to_send = (char*) malloc(BUFFER_LEN * sizeof(char));
            if(msg_to_send == NULL){
                detach_shared_memory(shared_var);
                error("[%s] LOG - send_message -> malloc(): ", get_time());
            }

            sprintf(msg_to_send, "[%s | %s]: %s", classes->classes[i].professor.username, class_name, message);
            if(sendto(sock, msg_to_send, BUFFER_LEN, 0, (struct sockaddr*)&addr, sizeof(addr)) == -1){
                detach_shared_memory(shared_var);
                error("[%s] LOG - send_message -> sendto(): ", get_time());
            }

            detach_shared_memory(shared_var);

            printf("[%s] LOG - Message sent to class '%s'. Message Content: '%s'\n", get_time(), class_name, msg_to_send);
            close(sock);
            return 1;
        }
    }
    detach_shared_memory(shared_var);
    return 0;
}

char* list_classes(){
    void* shared_var = attach_shared_memory(shmid);
    classes_list* classes = (classes_list*) shared_var;

    char* message = (char*) malloc(40 * classes->current_size * sizeof(char));
    if(message == NULL){
        detach_shared_memory(shared_var);
        error("[%s] LOG - list_classes() -> malloc(): ", get_time());
    }

    memset(message, 0, 40 * classes->current_size);
    for(int i = 0; i < classes->current_size; i++){
        char temp[40];
        memset(temp, 0, 40);
        sprintf(temp, "%s | %s\n", classes->classes[i].name, classes->classes[i].professor.username);
        strcat(message, temp);
    }

    detach_shared_memory(shared_var);

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

    char subscribed_classes[total_classes][40];

    for(int i = 0; i < classes->current_size; i++){
        for(int j = 0; j < classes->classes[i].current_size; j++){
            if(strcmp(classes->classes[i].students[j].username, user.username) == 0){
                char temp[40];
                memset(temp, 0, 40);
                sprintf(temp, "%s | %s\n", classes->classes[i].name, classes->classes[i].professor.username);
                strcpy(subscribed_classes[i], temp);
            }
        }
    }

    char* message = (char*) malloc((40*total_classes) * sizeof(char));
    if(message == NULL){
        detach_shared_memory(shared_var);
        error("[%s] LOG - list_subscribed() -> malloc(): ", get_time());
    }
    memset(message, 0, 40*total_classes);

    for(int i = 0; i < total_classes; i++){
        strcat(message, subscribed_classes[i]);
    }
    
    detach_shared_memory(shared_var);

    remove_line_break(message);

    return message;
}

char* generate_multicast_ip(){ 
    srand(time(NULL)); 
    
    int octet1 = 224 + rand() % 16;
    int octet2 = rand() % 256;
    int octet3 = rand() % 256;
    int octet4 = rand() % 256;

    char* multicast_ip = (char*) malloc(30 * sizeof(char));       
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

void handle_quit_server(){
    signal(SIGTERM, sig_handler);
    signal(SIGINT, sig_handler);

    queue_message received_msg;

    while(1){
        ssize_t nread = msgrcv(mq_id, &received_msg, sizeof(received_msg) - sizeof(long), 0, 0);
        if(nread < 0){
            continue;
        }

        // We want to capture msg == 1
        if(received_msg.msg == 1){
            // Send the message to the next process to avoid that he doesn´t receive the message
            msgsnd(mq_id, &received_msg, sizeof(received_msg) - sizeof(long), 0);
            break;
        }
    }

    // Start the countdown
    // Better implementation: the server should send these messages to clients.
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

    kill(getppid(), SIGTERM);
}

void sig_handler(int signum){
    if(signum == SIGTERM){
        kill(getppid(), SIGTERM);
        exit(EXIT_SUCCESS);
    }

    if(signum == SIGINT){
        exit(0);
    }
}