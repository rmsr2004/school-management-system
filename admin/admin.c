#include "admin.h"
#include "../functions/functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>

char** verify_admin_command(char* input, char* message){
    remove_line_break(input);
    char command[20];

    char* token = strtok(input, " ");
    if(token == NULL){
        strcpy(message, "COMMAND NOT VALID\n");
        return NULL;
    }
    strcpy(command, token);

    if(strcmp(command, "DEL") == 0){
        // DEL <username>
        char** args = create_args_array(1);
        token = strtok(NULL, " ");
        if(token == NULL){
            strcpy(message, "ERROR -> DEL <username>\n");
            strcpy(args[0], "ERROR");
            return args;
        }
        strcpy(args[0],token);        
        // verificar se tem argumentos a mais
        token = strtok(NULL, " ");
        if(token == NULL){
            // Não tem, então sucesso
            strcpy(message, "DEL\n");
            return args;
        }
        strcpy(message, "ERROR -> DEL <username>\n");
        strcpy(args[0],"ERROR");
        return args;
    } else if(strcmp(command, "ADD_USER") == 0){
        // ADD_USER <username> <password>
        char** args = create_args_array(3);
        token = strtok(NULL, " ");
        if(token == NULL){
            strcpy(message, "ERROR -> ADD_USER <username> <password> <administrador/aluno/professor>\n");
            strcpy(args[0],"ERROR");
            return args;
        }
        strcpy(args[0],token);
        token = strtok(NULL, " ");
        if(token == NULL){
            strcpy(message, "ERROR -> ADD_USER <username> <password> <administrador/aluno/professor>\n");
            strcpy(args[0],"ERROR");
            return args;
        }
        strcpy(args[1],token);
        token = strtok(NULL, " ");
        if(token == NULL){
            strcpy(message, "ERROR -> ADD_USER <username> <password> <administrador/aluno/professor>\n");
            strcpy(args[0],"ERROR");
            return args;
        }
        strcpy(args[2],token);
        // verificar se tem argumentos a mais
        token = strtok(NULL, " ");
        if(token == NULL){
            // Não tem, então sucesso
            strcpy(message, "ADD_USER\n");
            return args;
        }
        strcpy(message, "ERROR -> ADD_USER <username> <password> <administrador/aluno/professor>\n");
        strcpy(args[0],"ERROR");
        return args;
    } else if(strcmp(command, "LIST") == 0){
        // LIST
        char** args = create_args_array(1);
        token = strtok(NULL, " ");
        if(token == NULL){
            strcpy(message, "LIST\n");
            strcpy(args[0],"LIST");
            return args;
        }
        // se tiver argumentos, dá errado
        strcpy(message, "LIST não tem argumentos\n");
        strcpy(args[0],"ERROR");
        return args;
    } else if(strcmp(command, "QUIT_SERVER") == 0){
      char** args = create_args_array(1);
        // QUIT_SERVER
        token = strtok(NULL, " ");
        if(token == NULL){
            strcpy(message, "QUIT_SERVER\n");
            strcpy(args[0], "QUIT_SERVER");
            return args;
        }
        // se tiver argumentos, dá errado
        strcpy(message, "QUIT_SERVER não tem argumentos\n");
        strcpy(args[0],"ERROR");
        return args;
    } else if(strcmp(command, "HELP") == 0){
        char** args = create_args_array(1);
        // HELP
        token = strtok(NULL, " ");
        if(token == NULL){
            strcpy(message, "HELP\n");
            strcpy(args[0], "HELP");
            return args;
        }
        // se tiver argumentos, dá errado
        strcpy(message, "HELP não tem argumentos\n");
        strcpy(args[0],"ERROR");
        return args;
    } else {
        strcpy(message,"COMMAND NOT VALID\n");
        return NULL;
    }
}

char* help_admin(){
    char* message = (char*) malloc(150 * sizeof(char));

    sprintf(message, "Available commands:\nADD_USER <username> <password> <administrador/aluno/professor_\nDEL <username>\nLIST\nQUIT_SERVER\n");
    return message;
}

int add_user(char** args){
    char username[50];
    strcpy(username, args[0]);
    char password[50];
    strcpy(password, args[1]);
    char type[50];
    strcpy(type, args[2]);

    if(strcmp(type, "administrador") == 0 || strcmp(type, "aluno") == 0 || strcmp(type, "professor") == 0 ){
        FILE *f = fopen(config_file, "a");
        if(f == NULL) {
            error("[%d] LOG : add_user fopen(): ", get_time());
        }

        fprintf(f, "%s;%s;%s\n", username, password, type);

        if(fclose(f)){
            error("[%d] LOG : add_user fclose(): ", get_time());
        }
        return 1;
    }
    return 0;
}

int remove_user(char** args){
    char username[50];
    strcpy(username, args[0]);

    FILE *f = fopen(config_file, "r");
    FILE *temp_file = fopen("server/temp.txt", "w");

    if(f == NULL || temp_file == NULL){
        error("[%d] LOG : remove_user fopen(): ", get_time());
    }

    char line[100];
    char f_username[10], f_password[10], type[20];
    int user_found = 0;

    while(fgets(line, sizeof(line), f)){
        strcpy(f_username, strtok(line, ";"));
        strcpy(f_password, strtok(NULL, ";"));
        strcpy(type, strtok(NULL, ";"));

        remove_line_break(type);

        if(strcmp(f_username, username) != 0){
            fprintf(temp_file, "%s;%s;%s\n", f_username, f_password, type);
        } else{
            user_found = 1;
        }
    }

    fclose(f);
    fclose(temp_file);

    if(remove(config_file) != 0){
        error("[%d] LOG : remove_user remove(): ", get_time());
    }

    if(rename("server/temp.txt", config_file) != 0){
        error("[%d] LOG : remove_user rename(): ", get_time());
    }

    if(!user_found){
        return 0;
    }
    return 1;
}

char* list_users(){
    char* result_message = malloc(200 * sizeof(char));
    if(result_message == NULL){
        error("[%d] - LOG : list_users malloc(): ", get_time());
    }

    memset(result_message, 0, 200);
    FILE *f = fopen(config_file, "r");
    if(f == NULL){
        error("[%d] - LOG : list_users fopen(): ", get_time());
    }

    char line[100];
    char f_username[10], f_password[10], type[20];

    while(fgets(line, sizeof(line), f)){
        strcpy(f_username, strtok(line, ";"));
        strcpy(f_password, strtok(NULL, ";"));
        strcpy(type, strtok(NULL, ";"));

        remove_line_break(type);

        strcat(result_message, f_username);
        strcat(result_message, " [");
        strcat(result_message, type);
        strcat(result_message, "]\n");
    }
    fclose(f);
    return result_message;
}

void server_closing(){
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
}