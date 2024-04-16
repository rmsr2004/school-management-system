#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "../user/user.h"
#include "../users_list/users_list.h"

#define MAX_FILE_LEN 100

// Config file.
extern char config_file[10];

// Users list.
extern users_node* users_list;

/*
* Defines config_file variable to use globaly.
* @param filename File's filename.
*/
void define_config_file(char* filename);
/*
* Load users data from file.
* @return Users list.
*/
users_node* load_users_data();
/**
 * Verify login.
 * @param user Struct containing user login information
 * @param connection_type "UDP" or "TCP"
 * @return 1 if success, 0 otherwise
*/
int verify_login(struct_user user, char* connection_type);
/**
 * Invoke an error on system.
 * @param Message to describe the error.
*/
void error(const char *format, ...);
/* 
* Removes '\\n' from string.
*/
void remove_line_break(char* string);

#endif