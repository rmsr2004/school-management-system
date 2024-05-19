// João Afonso dos Santos Simões - 2022236316
// Rodrigo Miguel Santos Rodrigues - 2022233032
#include "functions.h"
#include "../shared_memory/shm_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <signal.h>

/*-----------------------------------------------------------------------------------------------------*
*                                       GLOBAL VARIABLES                                               *
*------------------------------------------------------------------------------------------------------*/
char* config_file;
int shmid;
pid_t tcp_pids[MAX_USERS];                                                         
int tcp_pids_index = 0;                  
pid_t tcp_pid;                             
pid_t udp_pid;                           
struct sockaddr_in server_udp;               
struct sockaddr_in server_tcp;                  
struct sockaddr_in client_tcp;                
int client_addr_size = sizeof(client_tcp);    
int tcp_socket;                                                              
int udp_socket;            
int recv_len;                    
char* input;                      
char* buffer;                    
struct sockaddr_in server;       
socklen_t slen = sizeof(server);   
int fd;                                      
int multicast_sock[CLASSES_SIZE];                
int multicast_index = 0;                      
struct ip_mreq multicast_mreq[CLASSES_SIZE];      
int logged_in = 0;                          
int last_port_used = 5000;                      
char* buffer;                                 
char* input;                  
pid_t pid;                               
pid_t multicast_pid[CLASSES_SIZE];         
pid_t father_pid;           
int client_id;                    
/*-----------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------------*
*                                            FUNCTIONS                                                 *
*------------------------------------------------------------------------------------------------------*/

void define_config_file(char* filename){
    config_file = (char*) malloc(50 * sizeof(char));
    strcpy(config_file, filename);
}

int verify_login(struct_user* user, char* connection_type){
    struct_user* new_user = get_user_from_file(user->username);
    if(new_user == NULL){
        return 0;
    }

    if(strcmp(new_user->password, user->password) != 0){
        return 0;
    }

    void* shared_var = attach_shared_memory(shmid);

    /*
    *   É login de um admin
    */
    if(strcmp(connection_type, "UDP") == 0){
        if(strcmp(new_user->type, "administrador") == 0){
            strcpy(user->type, "administrador");
            users_list* users = (users_list*)((char*) shared_var + sizeof(classes_list));
            for(int i = 0; i < users->current_size; i++){
                if(strcmp(users->users[i].type, "administrador") == 0){
                    detach_shared_memory(shared_var);
                    return 0;
                }
            }
            detach_shared_memory(shared_var);
            return 1;
        }
    } else{
        if(strcmp(new_user->type, "administrador") != 0){ 
            strcpy(user->type, new_user->type);
            /*
            * Verify if user is logged in.
            */
            
            users_list* users = (users_list*)((char*) shared_var + sizeof(classes_list));
            int index = find_user(users, user->username);
            if(index != -1){
                detach_shared_memory(shared_var);
                return 0;
            }
            detach_shared_memory(shared_var);
            return 1;
        }
    }
    return 0;
}

char* get_time(){
    time_t current_time;
    struct tm *timeinfo;
    char* str_time = (char*) malloc(sizeof(char) * 25);
    if(str_time == NULL){
        error("get_time -> malloc(): ");
    }

    time(&current_time);
    timeinfo = localtime(&current_time);

    sprintf(str_time, "%02d:%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    return str_time;
}

struct_user* get_user_from_file(char* username){
    FILE *f = fopen(config_file, "r");
    if(f == NULL){
        error("fopen(): ");
    }

    struct_user* user = (struct_user*) malloc(sizeof(struct_user));
    if(user == NULL){
        error("get_user_from_file -> malloc(): ");
    }

    char line[3*ARGS_LEN + 3], *token;
    while(fgets(line, sizeof(line), f) != NULL){
        token = strtok(line, ";");
        if(strcmp(token, username) == 0){
            strcpy(user->username, username);

            token = strtok(NULL, ";");
            strcpy(user->password, token);

            token = strtok(NULL, "\n");
            strcpy(user->type, token);

            return user;
        }
    }
    return NULL;
}

char** create_args_array(int args_number){
    char **args = malloc(args_number * sizeof(char*));
    if(args == NULL){
        printf("[%s] - LOG : create_args_array -> malloc(): ", get_time());
        return NULL;
    }

    for(int i = 0; i < args_number; i++){
        args[i] = malloc(ARGS_LEN * sizeof(char));

        if(args[i] == NULL){
            printf("[%s] - LOG : create_args_array -> args[i] malloc(): ", get_time());
            return NULL;
        }
    }
    return args;
}

int is_number(char *string){
    for(size_t i = 0; i < strlen(string); i++){
        if(string[i] < '0' || string[i] > '9'){
            return 0;
        }
    }
    return 1;
}

char* to_upper(char* string){
    char* copy = (char*) malloc(strlen(string) * sizeof(char));
    if(copy == NULL){
        error("to_upper -> malloc(): ");
    }
    strcpy(copy, string);

    for(size_t i = 0; i < strlen(copy); i++){
        copy[i] = toupper(copy[i]);
    }
    return copy;
}

int verify_string(char* string){
    remove_line_break(string);
    for(size_t i = 0; i < strlen(string); i++){
        if(string[i] >= 65 && string[i] <= 122){
            continue;
        }
        return 0;
    }
    return 1;
}

void error(const char *format, ...){
    va_list args;
    va_start(args, format);
    
    vfprintf(stderr, format, args);
    perror("");

    va_end(args);
    kill(getpid(), SIGINT);
    exit(-1);
}

void remove_line_break(char* string){
    int length = strlen(string);
    if(length == 0)
        return;
        
    for(int i = length - 1; i >= 0; i--){
        if(string[i] == '\n'){
            string[i] = '\0';
            return;
        }
    }
    return;
}