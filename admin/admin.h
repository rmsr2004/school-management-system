#ifndef ADMIN_H
#define ADMIN_H

#include "../user/user.h"

/*
* Verify if the commands received from admin are valid, i.e
* if they have all the arguments that are needed.
* @param input Command received.
* @param message Save message to send to admin or to inform what command was received.
* @return 1, if command is valid.
*         0, if command is a client command, but invalid.
*        -1, if command is not a client command.
*/
int verify_admin_command(char* input, char* message);

#endif