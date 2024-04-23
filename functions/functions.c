#include "functions.h"
#include "../user/user.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>

char config_file[10];
int mq_id;
key_t key;

void define_config_file(char* filename){
    strcpy(config_file, filename);
}

int verify_login(struct_user* user, char* connection_type){
    struct_user* new_user = get_user_from_file(user->username);
    if(new_user == NULL){
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
            strcpy(new_user->type, user->type);
            return 1;
        }
    }
    return 0;
}

char* get_time(){
    time_t current_time;
    struct tm *timeinfo;
    char* str_time = (char*) malloc(sizeof(char) * 9); // String para armazenar a hora no formato "HH:MM:SS"

    time(&current_time);
    timeinfo = localtime(&current_time);

    sprintf(str_time, "%02d:%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    return str_time;
}

struct_user* get_user_from_file(char* username){
    FILE *f = fopen(config_file, "r");
    if(f == NULL){
        error("fopen()");
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

void error(const char *format, ...){
    va_list args;
    va_start(args, format);
    
    vfprintf(stderr, format, args);
    perror("");

    va_end(args);
    exit(-1);
}

void remove_line_break(char* string){
  for(size_t i = 0; i < strlen(string); i++)
    if(string[i] == '\n')
      string[i] = '\0';
  return;
}