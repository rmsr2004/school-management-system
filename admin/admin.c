#include "admin.h"
#include "../functions/functions.h"
#include "../user/user.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

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
            strcpy(args[0],"ERROR");
            return args;
        }
        strcpy(args[0],token);        // verificar se tem argumentos a mais
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
    } else {
        strcpy(message,"COMMAND NOT VALID\n");
        return NULL;
    }
}


char* add_user(char** args){
    char* result_message = malloc(100 * sizeof(char)); // Alocar memória para a mensagem de resultado
    char username[50];
    strcpy(username, args[0]);
    char password[50];
    strcpy(password, args[1]);
    char type[50];
    strcpy(type, args[2]);

    if(strcmp(type, "administrador") == 0 || strcmp(type, "aluno") == 0 || strcmp(type, "professor") == 0 ){
        FILE *f = fopen(config_file, "a");
        if(f == NULL) {
            sprintf(result_message, "Erro a abrir o ficheiro de configuração!\n");
            return result_message;
        }

        fprintf(f, "%s;%s;%s\n", username, password, type);

        if(fclose(f)) {
            sprintf(result_message, "Erro a fechar o ficheiro de configuração!\n");
            return result_message;
        }
        sprintf(result_message, "Usuário '%s' adicionado com sucesso!\n", username);
    } else {
        sprintf(result_message, "Não foi possível adicionar o utilizador!\n");
    }

    return result_message;
}

char* remove_user(char** args) {
    char username[50];
    strcpy(username, args[0]);
    char* result_message = malloc(100 * sizeof(char)); // Alocar memória para a mensagem de resultado
    FILE *f = fopen(config_file, "r");
    FILE *temp_file = fopen("server/temp.txt", "w");

    if (f == NULL || temp_file == NULL) {
        sprintf(result_message, "Erro ao abrir arquivos!\n");
        return result_message;
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
        sprintf(result_message, "Erro ao remover arquivo original!\n");
        return result_message;
    }

    // Renomear o arquivo temporário para o nome do arquivo original
    if (rename("server/temp.txt", config_file) != 0) {
        sprintf(result_message, "Erro ao renomear arquivo temporário!\n");
        return result_message;
    }

    if (!user_found) {
        sprintf(result_message, "Usuário '%s' não encontrado no arquivo.\n", username);
    } else {
        sprintf(result_message, "Usuário '%s' removido com sucesso!\n", username);
    }


    return result_message;
}

char* list_users() {
    char* result_message = malloc(1000 * sizeof(char)); // Alocar memória para a mensagem de resultado
    FILE *f = fopen(config_file, "r");

    if (f == NULL) {
        perror("Erro a abrir ficheiro");
        sprintf(result_message, "Erro ao abrir o ficheiro de configuração!\n");
        return result_message;
    }

    char line[MAX_FILE_LEN];
    char f_username[10], f_password[10], type[20];

    sprintf(result_message, "Lista de Usuários:\n");

    while (fgets(line, sizeof(line), f)) {
        strcpy(f_username, strtok(line, ";"));
        strcpy(f_password, strtok(NULL, ";"));
        strcpy(type, strtok(NULL, ";"));

        remove_line_break(type);

        // Concatenar informações do usuário na mensagem de resultado
        strcat(result_message, "Usuário: ");
        strcat(result_message, f_username);
        strcat(result_message, " - Type: ");
        strcat(result_message, type);
        strcat(result_message, "\n");

    }
    fclose(f);

    return result_message;
}


char** create_args_array(int args_number){
  char **args = malloc(args_number * sizeof(char *)); // Aloca o array de ponteiros

  if (args == NULL) {
      printf("Erro ao alocar memória para o array de strings.\n");
      return NULL;
  }

  // Aloca memória para cada string individual e atribui ao array de ponteiros
  for (int i = 0; i < args_number; i++) {
      args[i] = malloc(100 * sizeof(char)); // Suponha que cada string terá no máximo 100 caracteres

      if (args[i] == NULL) {
          printf("Erro ao alocar memória para a string %d.\n", i);
          return NULL;
      }
    }

    return args;
}