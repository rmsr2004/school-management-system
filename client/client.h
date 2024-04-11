#ifndef CLIENT_H
#define CLIENT_H

/*
* Verify if the commands received from client are valid, i.e
* if they have all the arguments that are needed.
* @param input Command received.
* @param message Save message to send to client or to inform what command was received.
* @return 1, if command is valid.
*         0, if command is a client command, but invalid.
*        -1, if command is not a client command.
*/
int client_verify_command(char* input, char* message);

#endif