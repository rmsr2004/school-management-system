#include "functions.h"
#include "../user/user.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

char config_file[10];

void define_config_file(char* filename){
    strcpy(config_file, filename);
}

int verify_login(struct_user* user, char* connection_type){
    FILE *f = fopen(config_file, "r");

    if(f == NULL)
        error("Erro a abrir o ficheiro de configuração!");

    char line[100];
    char f_username[10], f_password[10], type[20];

    while(fgets(line, MAX_FILE_LEN-1, f) != NULL){
        strcpy(f_username, strtok(line, ";"));
        strcpy(f_password,  strtok(NULL, ";"));
        strcpy(type, strtok(NULL, ";"));

        remove_line_break(type);

        /*
        *   É login de um admin
        */
        if(strcmp(connection_type, "UDP") == 0){
            if(strcmp(f_username, user->username) == 0 && strcmp(f_password, user->password) == 0 && 
               strcmp(type, "administrador") == 0){

                if(fclose(f))
                    error("Erro a fechar o ficheiro de configuração!");
                strcpy(user->type, "administrador"); 
                return 1;
            }
        } else{
            if(strcmp(f_username, user->username) == 0 && strcmp(f_password, user->password) == 0 && 
               strcmp(type, "administrador") != 0){
                
                if(fclose(f))
                    error("Erro a fechar o ficheiro de configuração!");
                strcpy(user->type, "administrador"); 
                return 1;
            }
        }
    }
    if(fclose(f))
        error("Erro a fechar o ficheiro de configuração!");
    return 0;
}

void error(const char *format, ...){
    va_list args;
    va_start(args, format);
    
    printf("Error: ");
    vprintf(format, args);
    printf("\n");

    va_end(args);
    exit(-1);
}

void remove_line_break(char* string){
  for(size_t i = 0; i < strlen(string); i++)
    if(string[i] == '\n')
      string[i] = '\0';
  return;
}