/************************************************************************
 * CLIENTE liga ao servidor (definido em argv[1])                       *
 * no porto especificado (em argv[2])                                   *
 * USO: >client <server_ip> <port>                                      *
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include "../functions/functions.h"

#define BUF_SIZE    1024
#define INPUT_SIZE  100

int main(int argc, char* argv[]){
    struct sockaddr_in addr;
    struct hostent *hostPtr;

    if(argc != 3)
        error("Bad run! Tip: %s <host> <port>", argv[0]);

    if((hostPtr = gethostbyname(argv[1])) == 0)
        error("Não consegui obter endereço");

    bzero((void *) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ((struct in_addr *) (hostPtr->h_addr))->s_addr;
    addr.sin_port = htons((short) atoi(argv[2]));

    int fd;
    if((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
        error("Socket");
    
    if(connect(fd,(struct sockaddr *)&addr,sizeof (addr)) < 0)
        error("Connect");
        
    /*
    *   Command Line Interface
    */
    int nread = 0;
    char buffer[BUF_SIZE];
    
    char* input = (char*) malloc(INPUT_SIZE * sizeof(char));
    if(input == NULL)
        error("Erro a alocar memória!");
    
    while(1){
        /* enviar comando para o servidor */
        if(fgets(input, INPUT_SIZE - 1, stdin) == NULL)
            error("Erro a ler o comando");
        remove_line_break(input);

        //printf("Command sent: %s\n", input);

        write(fd, input, strlen(input));

        nread = read(fd, buffer, BUF_SIZE);
        
        if(nread < 0)
            continue;

        if(nread == 0)
            break;
        
        buffer[nread] = '\0';
        printf("%s\n", buffer);

        if(strcmp(buffer, "REJECTED\n") == 0 || strcmp(buffer, "LOGIN <username> <password>") == 0)
            exit(-1);
    }
    close(fd);
    return 0;
}