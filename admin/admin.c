// João Afonso dos Santos Simões - 2022236316
// Rodrigo Miguel Santos Rodrigues - 2022233032
#include "admin.h"
#include "../functions/functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>

/*-----------------------------------------------------------------------------------------------------*
*                                       FUNCTIONS THAT ADMIN HAVE                                      *
*------------------------------------------------------------------------------------------------------*/

char** verify_admin_command(char* input, char* message){
    remove_line_break(input);
    
    char command[ARGS_LEN];

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
        // check if there are more arguments
        token = strtok(NULL, " ");
        if(token == NULL){
            // No more arguments, success
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
        // check if there are more arguments
        token = strtok(NULL, " ");
        if(token == NULL){
            // No more arguments, success
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
        // if there are arguments, it's wrong
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
        // if there are arguments, it's wrong
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
        // if there are arguments, it's wrong
        strcpy(message, "HELP não tem argumentos\n");
        strcpy(args[0],"ERROR");
        return args;
    } else {
        strcpy(message,"COMMAND NOT VALID\n");
        return NULL;
    }
}

char* help_admin(){
    return "ADD_USER <username> <password> <administrador/aluno/professor>\nDEL <username>\nLIST\nQUIT_SERVER\n";
}

int register_user(char** args){
    char username[ARGS_LEN], password[ARGS_LEN], type[ARGS_LEN];
    
    strcpy(username, args[0]);
    strcpy(password, args[1]);
    strcpy(type, args[2]);

    if(strcmp(type, "administrador") == 0 || strcmp(type, "aluno") == 0 || strcmp(type, "professor") == 0 ){
        FILE *f = fopen(config_file, "a");
        if(f == NULL){
            error("[%d] LOG - add_user -> Error fopen(): ", get_time());
        }

        fprintf(f, "%s;%s;%s\n", username, password, type);

        if(fclose(f)){
            error("[%d] LOG - add_user -> Error fclose(): ", get_time());
        }
        return 1;
    }
    return 0;
}

int delete_user(char** args){
    char username[ARGS_LEN];
    strcpy(username, args[0]);

    FILE *f = fopen(config_file, "r");
    FILE *temp_file = fopen("server/temp.txt", "w"); // Temporary file to store the new file without the user to delete

    if(f == NULL || temp_file == NULL){
        error("[%d] LOG : remove_user fopen(): ", get_time());
    }

    char line[3*ARGS_LEN + 3];
    char f_username[ARGS_LEN], f_password[ARGS_LEN], type[ARGS_LEN];
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
    
    if(fclose(f) != 0 || fclose(temp_file) != 0){
        error("[%d] LOG - delete_user -> Error fclose(): ", get_time());
    }

    if(remove(config_file) != 0){
        error("[%d] LOG - delete_user -> Error remove(): ", get_time());
    }

    if(rename("server/temp.txt", config_file) != 0){
        error("[%d] LOG - delete_user -> Error remove(): ", get_time());
    }

    if(!user_found){
        return 0;
    }
    return 1;
}

char* list_users(){
    FILE* f = fopen(config_file, "r");
    if(f == NULL){
        error("[%d] LOG - list_users ->fopen(): ", get_time());
    }

    int total_users = 0; // Total number of users

    char line[3*ARGS_LEN + 3];
    while(fgets(line, sizeof(line), f)){
        total_users++;
    }
    fclose(f);

    char* result_message = malloc(total_users * (2*ARGS_LEN + 2) * sizeof(char));
    if(result_message == NULL){
        error("[%d] LOG - list_users -> Error malloc(): ", get_time());
    }
    memset(result_message, 0, total_users * (2*ARGS_LEN + 2));

    f = fopen(config_file, "r");
    if(f == NULL){
        error("[%d] LOG - list_users ->fopen(): ", get_time());
    }

    char username[ARGS_LEN], type[ARGS_LEN];
    while(fgets(line, sizeof(line), f)){
        strcpy(username, strtok(line, ";"));
        strtok(NULL, ";");
        strcpy(type, strtok(NULL, ";"));

        remove_line_break(type);

        strcat(result_message, username);
        strcat(result_message, " [");
        strcat(result_message, type);
        strcat(result_message, "]\n");
    }
    if(fclose(f) != 0){
        error("[%d] LOG - list_users -> Error fclose(): ", get_time());
    }

    return result_message;
}

/*-----------------------------------------------------------------------------------------------------*
*                                       UPD_ADMIN.C FUNCTIONS                                          *
*------------------------------------------------------------------------------------------------------*/

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

void sigint_handler(){
    if(input != NULL){
        free(input);
    }

    if(buffer != NULL){
        free(buffer);
    }

    char* message = malloc(18 * sizeof(char));
    sprintf(message, "ADMIN exiting...");
    sendto(udp_socket, message, (strlen(message)+1), 0, (struct sockaddr *)&server, sizeof(server));
    
    close(udp_socket);

    free(message);
    printf("\nExiting...\n");
    exit(0);
}

// admin.c