#ifndef CLIENT_H
#define CLIENT_H

#include "../user/user.h"
#include "../class/class.h"
#include <stdio.h>

/*
* Verify if the commands received from client are valid, i.e
* if they have all the arguments that are needed.
* @param input Command received.
* @param message Save message to send to admin or to inform what command was received.
* @return Array with arguments. If args[0] is "ERROR", the command is not valid.
*/
char** client_verify_command(char* input, struct_user user, char* message);
/*
* Returns the help message for the client.
*/
char* help_client(struct_user user);
/*
* Creates a new class.
*/
char* create_new_class(struct_user professor, char* name, int size);
/*
* Subscribe a student to a class.
*/
char* subscribe_class(struct_user user, char* class_name);
/*
* Send message to a class.
*/
int send_message(char* class_name, char* message);
/*
* List all classes.
*/
char* list_classes();
/*
* List all classes that the user is subscribed.
*/
char* list_subscribed(struct_user user);
/*
* Generates a random multicast ip;
*/
char* generate_multicast_ip();
/*
* Verify if the multicast ip is already in use.
*/
int ip_used(char* ip);
/*
* Handles the process that is responsible for reading messages from the message queue.
* If the message is 1 (queue_message.msg == 1), he is responsible to start a countdown and shuts 
* down the main process. 
*/
void handle_quit_server();
/*
* Handles the signum signal.
*/
void sig_handler(int signum);

#endif // CLIENT_H