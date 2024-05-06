#include "functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <signal.h>

char* config_file;
int mq_id;
key_t key_msg;
key_t key_shm;
int shmid;

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

    /*
    *   É login de um admin
    */
    if(strcmp(connection_type, "UDP") == 0){
        if(strcmp(new_user->type, "administrador") == 0){
            strcpy(user->type, "administrador"); 
            return 1;
        }
    } else{
        if(strcmp(new_user->type, "administrador") != 0){ 
            strcpy(user->type, new_user->type);
            return 1;
        }
    }
    return 0;
}

char* get_time(){
    time_t current_time;
    struct tm *timeinfo;
    char* str_time = (char*) malloc(sizeof(char) * 9);

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

    char line[INPUT_SIZE], *token;
    while(fgets(line, INPUT_SIZE-1, f) != NULL){
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
    char **args = malloc(args_number * sizeof(char *));
    if(args == NULL){
        printf("[%s] - LOG : create_args_array -> malloc(): ", get_time());
        return NULL;
    }

    for(int i = 0; i < args_number; i++){
        args[i] = malloc(20 * sizeof(char));

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

void to_upper(char* string){
    for(size_t i = 0; i < strlen(string); i++){
        string[i] = toupper(string[i]);
    }
    return;
}

int verify_string(char* string){
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