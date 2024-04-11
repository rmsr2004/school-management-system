#include "client.h"
#include "../functions/functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int client_verify_command(char* input, char* message){
    char command[20];
    char* token = strtok(input, " ");

    if(token == NULL){
        strcpy(message, "COMMAND NOT VALID\n");
        return -1;
    }
    strcpy(command, token);

    if(strcmp(command, "LIST_CLASSES") == 0){
        // LIST_CLASSES
        token = strtok(NULL, " ");
        if(token == NULL){
            strcpy(message, "LIST_CLASSES");
            return 1;
        }
        // se tiver argumentos, dá errado
        strcpy(message, "LIST_CLASSES não tem argumentos");
        return 0;
    } else if(strcmp(command, "LIST_SUBSCRIBED") == 0){
        // LIST_SUBSCRIBED
        token = strtok(NULL, " ");
        if(token == NULL){
            strcpy(message, "LIST_SUBSCRIBED");
            return 1;
        }
        // se tiver argumentos, dá errado
        strcpy(message, "LIST_SUBSCRIBED não tem argumentos");
        return 0;
    } else if(strcmp(command, "SUBSCRIBE_CLASS") == 0){
        // SUBSCRIBE_CLASS <name>
        // verificar se tem o argumento
        token = strtok(NULL, " ");
        if(token == NULL){
            strcpy(message, "ERROR -> SUBSCRIBE_CLASS <name>");
            return 0;
        }
        // verificar se tem argumentos a mais
        token = strtok(NULL, " ");
        if(token == NULL){
            // Não tem, então sucesso
            strcpy(message, "SUBSCRIBE_CLASS");
            return 1;
        }
        strcpy(message, "ERROR -> SUBSCRIBE_CLASS <name>");
        return 0;
    } else if(strcmp(command, "CREATE_CLASS") == 0){
        // CREATE_CLASS {name} {size}
        // verificar se tem os 2 argumentos
        token = strtok(NULL, " ");
        if(token == NULL){
            strcpy(message, "ERROR -> CREATE_CLASS {name} {size}");
            return 0;
        }
        token = strtok(NULL, " ");
        if(token == NULL){
            strcpy(message, "ERROR -> CREATE_CLASS {name} {size}");
            return 0;
        }
        // verificar se tem argumentos a mais
        token = strtok(NULL, " ");
        if(token == NULL){
            // Não tem, então sucesso
            strcpy(message, "CREATE_CLASS");
            return 1;
        }
        strcpy(message, "ERROR -> CREATE_CLASS {name} {size}");
        return 0;
    } else if(strcmp(command, "SEND") == 0){
        // SEND {name} {text that server will send to subscribers}
        // verificar se tem os 2 argumentos
        token = strtok(NULL, " ");
        if(token == NULL){
            strcpy(message, "ERROR -> SEND {name} {text that server will send to subscribers}");
            return 0;
        }
        token = strtok(NULL, " ");
        if(token == NULL){
            strcpy(message, "ERROR -> SEND {name} {text that server will send to subscribers}");
            return 0;
        }
        // verificar se tem argumentos a mais
        token = strtok(NULL, " ");
        if(token == NULL){
            // Não tem, então sucesso
            strcpy(message, "SEND");
            return 1;
        }
        strcpy(message, "ERROR -> SEND {name} {text that server will send to subscribers}");
        return 0;
    } else{
        strcpy(message, "COMMAND NOT VALID");
        return -1;
    }
}