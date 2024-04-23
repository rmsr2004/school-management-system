#ifndef CLIENT_H
#define CLIENT_H

#include "../user/user.h"

/*
* Verify if the commands received from client are valid, i.e
* if they have all the arguments that are needed.
* @param input Command received.
* @param message Save message to send to client or to inform what command was received.
* @return 1, if command is valid.
*         0, if command is a client command, but invalid.
*        -1, if command is not a client command.
*/
int client_verify_command(char* input, struct_user user, char* message);
/*
* Handles the process that is responsible for reading messages from the message queue.
* If the message is 1 (queue_message.msg == 1), he is responsible to start a countdown and shuts 
* down the main process. 
*/
void handle_quit_server();
/*
* Handles the signal.
*/
void sig_handler(int signum);
#endif