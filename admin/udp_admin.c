/**********************************************************************
 * CLIENTE liga ao servidor (definido em argv[1]) no porto especificado
 * (em argv[2])
 * USO: >cliente <enderecoServidor>  <porto>
 **********************************************************************/
// Rodrigo Miguel Santos Rodrigues - 2022233032
// João Afonso dos Santos Simões   - 2022236316 
#include "../functions/functions.h"
#include "../admin/admin.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>

int main(int argc, char* argv[]){
    if(argc != 3) error("Usage: %s <host> <port> \n", argv[0]);
    
    int s;
    struct sockaddr_in server;

    // Socket creation
    if((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        error("error na criação do socket");

    /* Set up the server name */
    server.sin_family      = AF_INET;                      /* Internet Domain    */
    server.sin_port        = htons(atoi(argv[2]));         /* Server Port        */
    server.sin_addr.s_addr = inet_addr(argv[1]);           /* Server's Address   */
    
    /*
        Command Line Interface
    */    
    int recv_len;

    // variável para guardar mensagem a enviar para o servidor
    char* input = (char*) malloc(INPUT_SIZE * sizeof(char)); 
    if(input == NULL) 
        error("Erro a alocar memória!");

    // variavel para guardar mensagem recebida do servidor 
    char buffer[BUFFER_LEN];
    socklen_t slen = sizeof(server);
    
    while(1){
        if(fgets(input, INPUT_SIZE-1, stdin) == NULL)
            error("Erro a ler do stdin");
        remove_line_break(input);

        // Enviar mensagem para o servidor
        if(sendto(s, input, (strlen(input)+1), 0, (struct sockaddr *)&server, sizeof(server)) < 0)
            error("sendto()");
        
        // Receber resposta do servidor
        if((recv_len = recvfrom(s, buffer, BUFFER_LEN, 0, (struct sockaddr *) &server, &slen)) == -1)
	        error("Erro no recvfrom");
        
        buffer[recv_len] = '\0';

        if(strcmp(buffer, "QUIT_SERVER\n") == 0){
            server_closing();
            exit(0);
        }

        // imprimir resposta do servidor
        printf("%s", buffer);

        // Se a resposta for "SAIR", sai do loop e a sessão é fechada
        if(strcmp(buffer, "REJECTED") == 0)
            break;
    }

    /* Deallocate the socket */
    close(s);
    return 0;
}