#ifndef ADMIN_H
#define ADMIN_H

#include "../user/user.h"

/*
* Verify if the commands received from admin are valid, i.e
* if they have all the arguments that are needed.
* @param input Command received.
* @param message Save message to send to admin or to inform what command was received.
* @return Array with arguments. If args[0] is "ERROR", the command is not valid.
*/
char** verify_admin_command(char* input, char* message);
/*
* Add user.
* @param args Array with username(args[0]), password(args[1]) and user type(args[2]).
*/
char* add_user(char** args);
/*
* Remove user with username args[0].
*/
char* remove_user(char** args);
/*
* List all users.
*/
char* list_users();
/*
* Create an arguments array with args_number length
*/
char** create_args_array(int args_number);

#endif