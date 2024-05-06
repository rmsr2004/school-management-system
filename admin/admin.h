#ifndef ADMIN_H
#define ADMIN_H

/*
* Verify if the commands received from admin are valid, i.e
* if they have all the arguments that are needed.
* @param input Command received.
* @param message Save message to send to admin or to inform what command was received.
* @return Array with arguments. If args[0] is "ERROR", the command is not valid.
*/
char** verify_admin_command(char* input, char* message);
/*
* Returns the help message for the admin.
*/
char* help_admin();
/*
* Add user.
* @param args Array with username(args[0]), password(args[1]) and user type(args[2]).
*/
int add_user(char** args);
/*
* Remove user with username args[0].
*/
int remove_user(char** args);
/*
* List all users.
*/
char* list_users();
/*
* Create an arguments array with args_number length
*/
char** create_args_array(int args_number);
/*
* Performs the closing of the server.
*/
void server_closing();

#endif