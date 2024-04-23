#include "client.h"
#include "../functions/functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/msg.h>

int client_verify_command(char* input, struct_user user, char* message){
    char command[20];
    char* token = strtok(input, " ");

    if(token == NULL){
        strcpy(message, "COMMAND NOT VALID\n");
        return -1;
    }
    strcpy(command, token);

    if(strcmp(command, "LIST_CLASSES") == 0 && strcmp(user.type, "aluno") == 0){
        // LIST_CLASSES
        token = strtok(NULL, " ");
        if(token == NULL){
            strcpy(message, "LIST_CLASSES");
            return 1;
        }
        // se tiver argumentos, dá errado
        strcpy(message, "LIST_CLASSES não tem argumentos");
        return 0;
    } else if(strcmp(command, "LIST_SUBSCRIBED") == 0 && strcmp(user.type, "aluno") == 0){
        // LIST_SUBSCRIBED
        token = strtok(NULL, " ");
        if(token == NULL){
            strcpy(message, "LIST_SUBSCRIBED");
            return 1;
        }
        // se tiver argumentos, dá errado
        strcpy(message, "LIST_SUBSCRIBED não tem argumentos");
        return 0;
    } else if(strcmp(command, "SUBSCRIBE_CLASS") == 0 && strcmp(user.type, "aluno") == 0){
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
    } else if(strcmp(command, "CREATE_CLASS") == 0 && strcmp(user.type, "professor") == 0){
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
    } else if(strcmp(command, "SEND") == 0 && strcmp(user.type, "professor") == 0){
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

void handle_quit_server(){
    signal(SIGTERM, sig_handler);

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
}