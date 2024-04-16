#include "admin.h"
#include "../functions/functions.h"
#include "../user/user.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

int verify_admin_command(char* input, char* message){
    remove_line_break(input);
    char command[20];  

    char* token = strtok(input, " ");
    if(token == NULL){
        strcpy(message, "COMMAND NOT VALID\n");
        return -1;
    }
    strcpy(command, token);
    
    if(strcmp(command, "DEL") == 0){
        // DEL <username>
        token = strtok(NULL, " ");
        if(token == NULL){
            strcpy(message, "ERROR -> DEL <username>\n");
            return 0;
        }
        // verificar se tem argumentos a mais
        token = strtok(NULL, " ");
        if(token == NULL){
            // Não tem, então sucesso
            strcpy(message, "DEL\n");
            return 1;
        }
        strcpy(message, "ERROR -> DEL <username>\n");
        return 0;
    } else if(strcmp(command, "ADD_USER") == 0){
        // ADD_USER <username> <password>
        token = strtok(NULL, " ");
        if(token == NULL){
            strcpy(message, "ERROR -> ADD_USER <username> <password> <administrador/aluno/professor>\n");
            return 0;
        }
        token = strtok(NULL, " ");
        if(token == NULL){
            strcpy(message, "ERROR -> ADD_USER <username> <password> <administrador/aluno/professor>\n");
            return 0;
        }
        token = strtok(NULL, " ");
        if(token == NULL){
            strcpy(message, "ERROR -> ADD_USER <username> <password> <administrador/aluno/professor>\n");
            return 0;
        }
        // verificar se tem argumentos a mais
        token = strtok(NULL, " ");
        if(token == NULL){
            // Não tem, então sucesso
            strcpy(message, "ADD_USER\n");
            return 1;
        }
        strcpy(message, "ERROR -> ADD_USER <username> <password> <administrador/aluno/professor>\n");
        return 0;
    } else if(strcmp(command, "LIST") == 0){
        // LIST
        token = strtok(NULL, " ");
        if(token == NULL){
            strcpy(message, "LIST\n");
            return 1;
        }
        // se tiver argumentos, dá errado
        strcpy(message, "LIST não tem argumentos\n");
        return 0;
    } else if(strcmp(command, "QUIT_SERVER") == 0){
        // QUIT_SERVER
        token = strtok(NULL, " ");
        if(token == NULL){
            strcpy(message, "QUIT_SERVER\n");
            return 1;
        }
        // se tiver argumentos, dá errado
        strcpy(message, "QUIT_SERVER não tem argumentos\n");
        return 0;
    } else {
        strcpy(message,"COMMAND NOT VALID\n");
        return -1;
    }
}
/*
void add_user(struct_user new_user){
    if(search_username(new_user.username)){
        printf("Username '%s' já registado!\n", new_user.username);
        return;
    }

    char type[50];
    strcpy(type, new_user->type);

    if(strcmp(type, "administrador") == 0 || strcmp(type, "aluno") == 0 || strcmp(type, "professor") == 0 ){
        FILE *f = fopen(config_file, "a");
        if(f == NULL)
            error("Erro a abrir o ficheiro de configuração!");
        
        fprintf(f, "%s;%s;%s\n", new_user.username, new_user.password, new_user.type);
        
        if(fclose(f))
            error("Erro a fechar o ficheiro de configuração!");
    } else{
        printf("Não foi possível adicionar o utilizador!\n");
        return;
    }
}

void remove_user(char* username) {
    FILE *f = fopen(config_file, "r");
    FILE *temp_file = fopen("temp.txt", "w");

    if (f == NULL || temp_file == NULL) {
        error("Erro ao abrir arquivos!");
    }

    char line[MAX_FILE_LEN];
    char f_username[10], f_password[10], type[20];
    int user_found = 0;

    while (fgets(line, sizeof(line), f)) {
        strcpy(f_username, strtok(line, ";"));
        strcpy(f_password, strtok(NULL, ";"));
        strcpy(type, strtok(NULL, ";"));

        remove_line_break(type);

        // Se o nome de usuário corresponder ao nome fornecido, não copie essa linha para o arquivo temporário
        if (strcmp(f_username, username) != 0) {
            fprintf(temp_file, "%s;%s;%s\n", f_username, f_password, type);
        } else {
            user_found = 1; // Indica que o usuário foi encontrado
        }
    }

    fclose(f);
    fclose(temp_file);

    // Remover o arquivo original
    if (remove(config_file) != 0) {
        error("Erro ao remover arquivo original!");
    }

    // Renomear o arquivo temporário para o nome do arquivo original
    if (rename("temp.txt", config_file) != 0) {
        error("Erro ao renomear arquivo temporário!");
    }

    if (!user_found) {
        printf("Usuário '%s' não encontrado no arquivo.\n", username);
    }
}
int search_username(char* username){
    FILE *f = fopen(config_file, "r");
    if(f == NULL)
        error("Erro a abrir o ficheiro de configuração!");
    
    char line[100];
    while(fgets(line, MAX_FILE_LEN-1, f) != NULL){
        strcpy(line, strtok(line, ";"));

        if(strcmp(line, username) == 0){
            if(fclose(f))
                error("Erro a fechar o ficheiro de configuração!");
            return 1;
        }
    }
    return 0;
}
void list_users() {
    FILE *f = fopen(config_file, "r");

    if (f == NULL) {
        error("Erro ao abrir o ficheiro de configuração!");
    }

    char line[MAX_FILE_LEN];
    char f_username[10], f_password[10], type[20];

    while (fgets(line, sizeof(line), f)) {
        strcpy(f_username, strtok(line, ";"));
        strcpy(f_password, strtok(NULL, ";"));
        strcpy(type, strtok(NULL, ";"));

        remove_line_break(type);

        
        printf("Usuário: %s\n", f_username);
        
    }
    fclose(f);
}
*/